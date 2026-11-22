#include <fudge.h>
#include <abi.h>
#include <disk.h>

static struct message_blockinfo blockinfo;

static unsigned int block;
static unsigned int partsector;
static unsigned int mode;

#define CACHESLOTS 4
#define EXTENTROOTSIZE 60

struct cacheslot
{

    unsigned int sector;
    unsigned int count;
    char data[EXT2_MAXBLOCKSIZE];

};

struct run
{

    unsigned int sector;
    unsigned int blocks;

};

struct extentroot
{

    unsigned char data[EXTENTROOTSIZE];

};

struct extentleaf
{

    struct ext2_extentheader *header;
    unsigned int block;
    unsigned int first;

};

struct span
{

    unsigned int sector;
    unsigned int blocks;
    unsigned int count;

};

static struct cacheslot cache[CACHESLOTS];
static unsigned int cachenext;

static void updatecache(unsigned int sector, unsigned int count)
{

    unsigned int sectors = count / MESSAGE_SECTORSIZE;
    unsigned int i;

    for (i = 0; i < CACHESLOTS; i++)
    {

        struct cacheslot *slot = &cache[i];
        unsigned int slotsectors = slot->count / MESSAGE_SECTORSIZE;

        if (slot->sector + slotsectors <= sector || slot->sector >= sector + sectors)
            continue;

        if (slot->sector >= sector && slot->sector + slotsectors <= sector + sectors)
            buffer_copy(slot->data, (char *)blockinfo.buffer + (slot->sector - sector) * MESSAGE_SECTORSIZE, slot->count);
        else
            slot->count = 0;

    }

}

static unsigned int getdisksector(unsigned int sector, unsigned int blocksize)
{

    return sector * (blocksize / MESSAGE_SECTORSIZE);

}

static unsigned int sendblockrequest(unsigned int event, unsigned int reply, unsigned int count, unsigned int sector, unsigned int blocksize, unsigned int target)
{

    struct message_blockrequest request;

    request.sector = partsector + getdisksector(sector, blocksize);
    request.count = count;
    request.target = target;

    if (channel_send(1, block, event, sizeof (struct message_blockrequest), &request))
    {

        struct message_blockresponse response;

        return (channel_wait(1, block, reply, sizeof (struct message_blockresponse), &response)) ? response.count : 0;

    }

    return 0;

}

static unsigned int sendblockreadrequest(unsigned int count, unsigned int sector, unsigned int blocksize)
{

    return sendblockrequest(MESSAGE_TYPE_BLOCKREADREQUEST, MESSAGE_TYPE_BLOCKREADRESPONSE, count, sector, blocksize, 0);

}

static unsigned int sendblockwriterequest(unsigned int count, unsigned int sector, unsigned int blocksize)
{

    unsigned int written = sendblockrequest(MESSAGE_TYPE_BLOCKWRITEREQUEST, MESSAGE_TYPE_BLOCKWRITERESPONSE, count, sector, blocksize, 0);

    updatecache(getdisksector(sector, blocksize), written);

    return written;

}

static struct ext2_superblock sb;
static unsigned int blocksize;
static unsigned int descsize;
static unsigned int csumseed;

static char *readmeta(unsigned int sector)
{

    unsigned int first = getdisksector(sector, blocksize);
    struct cacheslot *slot;
    unsigned int i;

    for (i = 0; i < CACHESLOTS; i++)
    {

        slot = &cache[i];

        if (slot->count == blocksize && slot->sector == first)
            return slot->data;

    }

    if (sendblockreadrequest(blocksize, sector, blocksize) != blocksize || blocksize > EXT2_MAXBLOCKSIZE)
        return (char *)blockinfo.buffer;

    slot = &cache[cachenext];
    cachenext = (cachenext + 1) % CACHESLOTS;

    buffer_copy(slot->data, (void *)blockinfo.buffer, blocksize);

    slot->sector = first;
    slot->count = blocksize;

    return slot->data;

}

static void readsuperblock(void)
{

    sendblockreadrequest(1024, 1, 1024);
    buffer_copy(&sb, (void *)blockinfo.buffer, sizeof (struct ext2_superblock));
    ext2_convertsuperblock(&sb);

    blocksize = (1024 << sb.blockSize);
    descsize = (sb.requiredFeatures & EXT2_INCOMPAT_64BIT) ? sb.descSize : 32;
    csumseed = (sb.requiredFeatures & EXT2_INCOMPAT_CSUMSEED) ? ext2_read32((char *)blockinfo.buffer + EXT2_SUPERBLOCK_SEED) : ext2_crc32c(0xFFFFFFFF, sb.system, 16);

}

static unsigned int hasmetadatachecksums(void)
{

    return sb.unsupportedFeatures & EXT2_ROCOMPAT_METADATACSUM;

}

static unsigned int hasgroupchecksums(void)
{

    return sb.unsupportedFeatures & (EXT2_ROCOMPAT_GROUPCSUM | EXT2_ROCOMPAT_METADATACSUM);

}

static unsigned int getnodeseed(unsigned int id, unsigned int generation)
{

    unsigned char number[4];
    unsigned int crc;

    ext2_write32(number, id);

    crc = ext2_crc32c(csumseed, number, 4);

    ext2_write32(number, generation);

    return ext2_crc32c(crc, number, 4);

}

static void writesuperblock(void)
{

    sendblockreadrequest(1024, 1, 1024);
    buffer_copy((void *)blockinfo.buffer, &sb, sizeof (struct ext2_superblock));
    ext2_convertsuperblock((struct ext2_superblock *)blockinfo.buffer);

    if (hasmetadatachecksums())
        ext2_write32((char *)blockinfo.buffer + EXT2_SUPERBLOCK_CHECKSUM, ext2_crc32c(0xFFFFFFFF, (void *)blockinfo.buffer, EXT2_SUPERBLOCK_CHECKSUM));

    sendblockwriterequest(1024, 1, 1024);

}

static void readblockgroup(struct ext2_blockgroup *bg, unsigned int index)
{

    unsigned int slots = blocksize / descsize;
    unsigned int sector = sb.superblockIndex + 1 + index / slots;
    unsigned int offset = (index % slots) * descsize;

    buffer_clear(bg, sizeof (struct ext2_blockgroup));
    buffer_copy(bg, readmeta(sector) + offset, descsize);
    ext2_convertblockgroup(bg);

}

static void writeblockgroup(struct ext2_blockgroup *bg, unsigned int index)
{

    unsigned int slots = blocksize / descsize;
    unsigned int sector = sb.superblockIndex + 1 + index / slots;
    unsigned int offset = (index % slots) * descsize;
    char *descriptor = (char *)blockinfo.buffer + offset;
    struct ext2_blockgroup raw;

    buffer_copy(&raw, bg, sizeof (struct ext2_blockgroup));
    ext2_convertblockgroup(&raw);
    sendblockreadrequest(blocksize, sector, blocksize);
    buffer_copy(descriptor, &raw, descsize);

    if (hasgroupchecksums())
    {

        unsigned char number[4];
        unsigned int crc;

        ext2_write32(number, index);

        if (hasmetadatachecksums())
        {

            ext2_write16(descriptor + EXT2_GROUP_CHECKSUM, 0);

            crc = ext2_crc32c(ext2_crc32c(csumseed, number, 4), descriptor, descsize);

        }

        else
        {

            crc = ext2_crc16(ext2_crc16(ext2_crc16(0xFFFF, sb.system, 16), number, 4), descriptor, EXT2_GROUP_CHECKSUM);
            crc = ext2_crc16(crc, descriptor + EXT2_GROUP_CHECKSUM + 2, descsize - EXT2_GROUP_CHECKSUM - 2);

        }

        ext2_write16(descriptor + EXT2_GROUP_CHECKSUM, crc);

    }

    sendblockwriterequest(blocksize, sector, blocksize);

}

static void readnode(struct ext2_node *node, unsigned int start, unsigned int index)
{

    unsigned int slots = blocksize / sb.nodeSize;
    unsigned int sector = start + index / slots;
    unsigned int offset = (index % slots) * sb.nodeSize;

    buffer_copy(node, readmeta(sector) + offset, sizeof (struct ext2_node));
    ext2_convertnode(node);

}

static unsigned int getextrasize(void)
{

    unsigned int size = (sb.wantExtraSize) ? sb.wantExtraSize : EXT2_NODE_EXTRADEFAULT;

    if (sb.nodeSize <= EXT2_NODE_BASESIZE || !(sb.unsupportedFeatures & (EXT2_ROCOMPAT_EXTRASIZE | EXT2_ROCOMPAT_METADATACSUM)))
        return 0;

    return (size > sb.nodeSize - EXT2_NODE_BASESIZE) ? sb.nodeSize - EXT2_NODE_BASESIZE : size;

}

static void setnodechecksum(char *raw, unsigned int id)
{

    unsigned int high = sb.nodeSize > EXT2_NODE_BASESIZE && ext2_read16(raw + EXT2_NODE_EXTRASIZE) >= EXT2_NODE_CHECKSUMHIGH + 2 - EXT2_NODE_BASESIZE;
    unsigned int crc;

    ext2_write16(raw + EXT2_NODE_CHECKSUMLOW, 0);

    if (high)
        ext2_write16(raw + EXT2_NODE_CHECKSUMHIGH, 0);

    crc = ext2_crc32c(getnodeseed(id, ext2_read32(raw + EXT2_NODE_GENERATION)), raw, sb.nodeSize);

    ext2_write16(raw + EXT2_NODE_CHECKSUMLOW, crc);

    if (high)
        ext2_write16(raw + EXT2_NODE_CHECKSUMHIGH, crc >> 16);

}

static void writenode(struct ext2_node *node, unsigned int start, unsigned int index, unsigned int id, unsigned int fresh)
{

    unsigned int slots = blocksize / sb.nodeSize;
    unsigned int sector = start + index / slots;
    char *raw = (char *)blockinfo.buffer + (index % slots) * sb.nodeSize;

    sendblockreadrequest(blocksize, sector, blocksize);

    if (fresh)
    {

        buffer_clear(raw, sb.nodeSize);

        if (getextrasize())
            ext2_write16(raw + EXT2_NODE_EXTRASIZE, getextrasize());

    }

    buffer_copy(raw, node, sizeof (struct ext2_node));
    ext2_convertnode((struct ext2_node *)raw);

    if (hasmetadatachecksums())
        setnodechecksum(raw, id);

    sendblockwriterequest(blocksize, sector, blocksize);

}

static void simpleread(struct ext2_node *node, unsigned int id)
{

    unsigned int igroup = (id - 1) / sb.nodeCountGroup;
    unsigned int inode = (id - 1) % sb.nodeCountGroup;
    struct ext2_blockgroup bg;

    readblockgroup(&bg, igroup);
    readnode(node, bg.blockTableAddress, inode);

}

static void simplewrite(struct ext2_node *node, unsigned int id)
{

    unsigned int igroup = (id - 1) / sb.nodeCountGroup;
    unsigned int inode = (id - 1) % sb.nodeCountGroup;
    struct ext2_blockgroup bg;

    readblockgroup(&bg, igroup);
    writenode(node, bg.blockTableAddress, inode, id, 0);

}

static void simplecreate(struct ext2_node *node, unsigned int id)
{

    unsigned int igroup = (id - 1) / sb.nodeCountGroup;
    unsigned int inode = (id - 1) % sb.nodeCountGroup;
    struct ext2_blockgroup bg;

    readblockgroup(&bg, igroup);
    writenode(node, bg.blockTableAddress, inode, id, 1);

}

static void setblockusagechecksum(struct ext2_blockgroup *bg, unsigned char *bitmap)
{

    if (hasmetadatachecksums())
    {

        unsigned int crc = ext2_crc32c(csumseed, bitmap, sb.blockCountGroup / 8);

        bg->blockUsageChecksum = crc & 0xFFFF;
        bg->blockUsageChecksumHigh = crc >> 16;

    }

}

static void setnodeusagechecksum(struct ext2_blockgroup *bg, unsigned char *bitmap)
{

    if (hasmetadatachecksums())
    {

        unsigned int crc = ext2_crc32c(csumseed, bitmap, sb.nodeCountGroup / 8);

        bg->nodeUsageChecksum = crc & 0xFFFF;
        bg->nodeUsageChecksumHigh = crc >> 16;

    }

}

static unsigned int allocblockgroup(unsigned int blockgroup)
{

    struct ext2_blockgroup bg;

    readblockgroup(&bg, blockgroup);

    if (hasgroupchecksums() && (bg.flags & EXT2_GROUP_BLOCKUNINIT))
        return 0;

    if (bg.blockCountUnalloc)
    {

        unsigned char *bitmap = (unsigned char *)blockinfo.buffer;
        unsigned int i;

        sendblockreadrequest(blocksize, bg.blockUsageAddress, blocksize);

        for (i = 0; i < sb.blockCountGroup; i++)
        {

            unsigned int index = i / 8;
            unsigned int bit = 1 << (i % 8);

            if (!(bitmap[index] & bit))
            {

                bitmap[index] |= bit;

                sendblockwriterequest(blocksize, bg.blockUsageAddress, blocksize);
                setblockusagechecksum(&bg, bitmap);

                bg.blockCountUnalloc--;

                writeblockgroup(&bg, blockgroup);

                sb.blockCountUnalloc--;

                writesuperblock();

                return sb.superblockIndex + blockgroup * sb.blockCountGroup + i;

            }

        }

    }

    return 0;

}

static unsigned int allocnodegroup(unsigned int igroup)
{

    struct ext2_blockgroup bg;

    readblockgroup(&bg, igroup);

    if (hasgroupchecksums() && (bg.flags & EXT2_GROUP_NODEUNINIT))
        return 0;

    if (bg.nodeCountUnalloc)
    {

        unsigned char *bitmap = (unsigned char *)blockinfo.buffer;
        unsigned int i;

        sendblockreadrequest(blocksize, bg.nodeUsageAddress, blocksize);

        for (i = 0; i < sb.nodeCountGroup; i++)
        {

            unsigned int index = i / 8;
            unsigned int bit = 1 << (i % 8);

            if (!(bitmap[index] & bit))
            {

                bitmap[index] |= bit;

                sendblockwriterequest(blocksize, bg.nodeUsageAddress, blocksize);
                setnodeusagechecksum(&bg, bitmap);

                bg.nodeCountUnalloc--;

                if (hasgroupchecksums() && i >= sb.nodeCountGroup - bg.nodeCountUnused)
                    bg.nodeCountUnused = sb.nodeCountGroup - i - 1;

                writeblockgroup(&bg, igroup);

                sb.nodeCountUnalloc--;

                writesuperblock();

                return igroup * sb.nodeCountGroup + i + 1;

            }

        }

    }

    return 0;

}

static unsigned int getgroupcount(void)
{

    return (sb.blockCount - sb.superblockIndex + sb.blockCountGroup - 1) / sb.blockCountGroup;

}

static unsigned int allocblock(unsigned int blockgroup)
{

    unsigned int count = getgroupcount();
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        unsigned int block = allocblockgroup((blockgroup + i) % count);

        if (block)
            return block;

    }

    return 0;

}

static unsigned int allocnode(unsigned int igroup)
{

    unsigned int count = getgroupcount();
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        unsigned int id = allocnodegroup((igroup + i) % count);

        if (id)
            return id;

    }

    return 0;

}

static void freeblock(unsigned int block)
{

    unsigned int igroup = (block - sb.superblockIndex) / sb.blockCountGroup;
    unsigned int i = (block - sb.superblockIndex) % sb.blockCountGroup;
    unsigned char *bitmap = (unsigned char *)blockinfo.buffer;
    struct ext2_blockgroup bg;

    readblockgroup(&bg, igroup);
    sendblockreadrequest(blocksize, bg.blockUsageAddress, blocksize);

    bitmap[i / 8] &= ~(1 << (i % 8));

    sendblockwriterequest(blocksize, bg.blockUsageAddress, blocksize);
    setblockusagechecksum(&bg, bitmap);

    bg.blockCountUnalloc++;

    writeblockgroup(&bg, igroup);

    sb.blockCountUnalloc++;

    writesuperblock();

}

static void freenode(unsigned int id, unsigned int directory)
{

    unsigned int igroup = (id - 1) / sb.nodeCountGroup;
    unsigned int i = (id - 1) % sb.nodeCountGroup;
    unsigned char *bitmap = (unsigned char *)blockinfo.buffer;
    struct ext2_blockgroup bg;

    readblockgroup(&bg, igroup);
    sendblockreadrequest(blocksize, bg.nodeUsageAddress, blocksize);

    bitmap[i / 8] &= ~(1 << (i % 8));

    sendblockwriterequest(blocksize, bg.nodeUsageAddress, blocksize);
    setnodeusagechecksum(&bg, bitmap);

    bg.nodeCountUnalloc++;

    if (directory)
        bg.directoryCount--;

    writeblockgroup(&bg, igroup);

    sb.nodeCountUnalloc++;

    writesuperblock();

}

static unsigned int getindirect(unsigned int sector, unsigned int index)
{

    if (sector)
    {

        unsigned int *table = (unsigned int *)readmeta(sector);

        return ext2_read32(&table[index]);

    }

    return 0;

}

static void getextentroot(struct ext2_node *node, struct extentroot *root)
{

    unsigned int i;

    for (i = 0; i < 12; i++)
        ext2_write32(root->data + i * 4, node->pointer[i]);

    ext2_write32(root->data + 48, node->singlyIndirectPointer);
    ext2_write32(root->data + 52, node->doublyIndirectPointer);
    ext2_write32(root->data + 56, node->tripplyIndirectPointer);

}

static unsigned int isextentheader(struct ext2_extentheader *header, unsigned int size)
{

    unsigned int max = ext2_read16(&header->max);

    return ext2_read16(&header->magic) == EXT2_EXTENTMAGIC && ext2_read16(&header->entries) <= max && max <= (size - sizeof (struct ext2_extentheader)) / sizeof (struct ext2_extent) && ext2_read16(&header->depth) <= EXT2_EXTENTMAXDEPTH;

}

static struct extentleaf findextentleaf(struct ext2_extentheader *header, unsigned int index)
{

    unsigned int size = EXTENTROOTSIZE;
    struct extentleaf leaf;
    unsigned int level;

    leaf.header = 0;
    leaf.block = 0;
    leaf.first = 0;

    for (level = 0; isextentheader(header, size) && ext2_read16(&header->depth); level++)
    {

        struct ext2_extentindex *entry = (struct ext2_extentindex *)(header + 1);
        unsigned int entries = ext2_read16(&header->entries);
        unsigned int i;

        if (!entries)
            return leaf;

        for (i = 1; i < entries && ext2_read32(&(entry + 1)->block) <= index; i++)
            entry++;

        if (level == EXT2_EXTENTMAXDEPTH || ext2_read16(&entry->leafHigh))
            return leaf;

        leaf.block = ext2_read32(&entry->leaf);
        leaf.first = ext2_read32(&entry->block);
        header = (struct ext2_extentheader *)readmeta(leaf.block);
        size = blocksize;

    }

    if (isextentheader(header, size))
        leaf.header = header;

    return leaf;

}

static struct run getextentrun(struct ext2_node *node, unsigned int index, unsigned int max)
{

    struct extentroot root;
    struct ext2_extentheader *header;
    struct run run;

    getextentroot(node, &root);

    header = findextentleaf((struct ext2_extentheader *)root.data, index).header;

    run.sector = 0;
    run.blocks = 1;

    if (header && !ext2_read16(&header->depth))
    {

        struct ext2_extent *extent = (struct ext2_extent *)(header + 1);
        unsigned int entries = ext2_read16(&header->entries);
        unsigned int i;

        for (i = 0; i < entries; i++, extent++)
        {

            unsigned int block = ext2_read32(&extent->block);
            unsigned int length = ext2_read16(&extent->length);
            unsigned int initialized = length <= EXT2_EXTENTMAXLENGTH;

            if (!initialized)
                length -= EXT2_EXTENTMAXLENGTH;

            if (index < block)
            {

                run.blocks = block - index;

                break;

            }

            if (index - block < length)
            {

                if (initialized && !ext2_read16(&extent->startHigh))
                    run.sector = ext2_read32(&extent->start) + index - block;

                run.blocks = length - (index - block);

                break;

            }

        }

    }

    if (run.blocks > max)
        run.blocks = max;

    return run;

}

static void setextentroot(struct ext2_node *node, struct extentroot *root)
{

    unsigned int i;

    for (i = 0; i < 12; i++)
        node->pointer[i] = ext2_read32(root->data + i * 4);

    node->singlyIndirectPointer = ext2_read32(root->data + 48);
    node->doublyIndirectPointer = ext2_read32(root->data + 52);
    node->tripplyIndirectPointer = ext2_read32(root->data + 56);

}

static unsigned int getextentlength(struct ext2_extent *extent)
{

    unsigned int length = ext2_read16(&extent->length);

    return (length > EXT2_EXTENTMAXLENGTH) ? length - EXT2_EXTENTMAXLENGTH : length;

}

static unsigned int insertextent(struct ext2_extentheader *header, unsigned int index, unsigned int sector)
{

    struct ext2_extent *first = (struct ext2_extent *)(header + 1);
    struct ext2_extent *extent = first;
    unsigned int entries = ext2_read16(&header->entries);
    unsigned int position = 0;
    unsigned int i;

    while (position < entries && ext2_read32(&extent->block) <= index)
    {

        position++;
        extent++;

    }

    if (position)
    {

        struct ext2_extent *previous = extent - 1;
        unsigned int block = ext2_read32(&previous->block);
        unsigned int length = ext2_read16(&previous->length);

        if (index - block < getextentlength(previous))
            return 0;

        if (length < EXT2_EXTENTMAXLENGTH && block + length == index && !ext2_read16(&previous->startHigh) && ext2_read32(&previous->start) + length == sector)
        {

            ext2_write16(&previous->length, length + 1);

            return 1;

        }

    }

    if (entries == ext2_read16(&header->max))
        return 0;

    for (i = entries; i > position; i--)
        buffer_copy(first + i, first + i - 1, sizeof (struct ext2_extent));

    ext2_write32(&extent->block, index);
    ext2_write16(&extent->length, 1);
    ext2_write16(&extent->startHigh, 0);
    ext2_write32(&extent->start, sector);
    ext2_write16(&header->entries, entries + 1);

    return 1;

}

static void setextentchecksum(char *block, struct ext2_node *node, unsigned int id)
{

    struct ext2_extentheader *header = (struct ext2_extentheader *)block;
    unsigned int size = sizeof (struct ext2_extentheader) + ext2_read16(&header->max) * sizeof (struct ext2_extent);

    if (hasmetadatachecksums() && size + 4 <= blocksize)
        ext2_write32(block + size, ext2_crc32c(getnodeseed(id, node->generation), block, size));

}

static unsigned int allocextent(struct ext2_node *node, unsigned int index, unsigned int id)
{

    struct extentroot root;
    struct extentleaf leaf;
    char data[EXT2_MAXBLOCKSIZE];
    unsigned int sector;

    getextentroot(node, &root);

    leaf = findextentleaf((struct ext2_extentheader *)root.data, index);

    if (!leaf.header || ext2_read16(&leaf.header->depth) || index < leaf.first)
        return 0;

    if (leaf.block)
        buffer_copy(data, leaf.header, blocksize);

    sector = allocblock((id - 1) / sb.nodeCountGroup);

    if (!sector)
        return 0;

    if (!insertextent((leaf.block) ? (struct ext2_extentheader *)data : (struct ext2_extentheader *)root.data, index, sector))
    {

        freeblock(sector);

        return 0;

    }

    if (leaf.block)
    {

        buffer_copy((void *)blockinfo.buffer, data, blocksize);
        setextentchecksum((char *)blockinfo.buffer, node, id);
        sendblockwriterequest(blocksize, leaf.block, blocksize);

    }

    else
    {

        setextentroot(node, &root);

    }

    node->sectorCount += blocksize / 512;

    return sector;

}

static struct run getrun(struct ext2_node *node, unsigned int offset, unsigned int max)
{

    unsigned int index = offset / blocksize;
    unsigned int slots = blocksize / 4;
    unsigned int direct = index < 12;
    unsigned int *entries;
    unsigned int available;
    unsigned int first;
    struct run run;

    if (node->flags & EXT2_NODE_EXTENTS)
        return getextentrun(node, index, max);

    run.sector = 0;
    run.blocks = 1;

    if (direct)
    {

        entries = node->pointer + index;
        available = 12 - index;

    }

    else
    {

        unsigned int table;

        index -= 12;

        if (index < slots)
        {

            table = node->singlyIndirectPointer;

        }

        else if (index - slots < slots * slots)
        {

            index -= slots;
            table = getindirect(node->doublyIndirectPointer, index / slots);

        }

        else
        {

            index -= slots + slots * slots;
            table = getindirect(getindirect(node->tripplyIndirectPointer, index / (slots * slots)), (index / slots) % slots);

        }

        if (!table)
            return run;

        entries = (unsigned int *)readmeta(table) + index % slots;
        available = slots - index % slots;

    }

    if (available > max)
        available = max;

    first = (direct) ? entries[0] : ext2_read32(entries);

    for (run.blocks = 1; run.blocks < available; run.blocks++)
    {

        unsigned int next = (direct) ? entries[run.blocks] : ext2_read32(&entries[run.blocks]);

        if (next != ((first) ? first + run.blocks : 0))
            break;

    }

    run.sector = first;

    return run;

}

static unsigned int getsector(struct ext2_node *node, unsigned int offset)
{

    return getrun(node, offset, 1).sector;

}

static unsigned int alloctable(struct ext2_node *node, unsigned int igroup)
{

    unsigned int block = allocblock(igroup);

    if (block)
    {

        buffer_clear((void *)blockinfo.buffer, blocksize);
        sendblockwriterequest(blocksize, block, blocksize);

        node->sectorCount += blocksize / 512;

    }

    return block;

}

static unsigned int allocentry(struct ext2_node *node, unsigned int table, unsigned int index, unsigned int istable, unsigned int igroup)
{

    unsigned int *entries = (unsigned int *)blockinfo.buffer;

    sendblockreadrequest(blocksize, table, blocksize);

    if (!ext2_read32(&entries[index]))
    {

        unsigned int block;

        if (istable)
        {

            block = alloctable(node, igroup);

        }

        else
        {

            block = allocblock(igroup);

            if (block)
                node->sectorCount += blocksize / 512;

        }

        if (block)
        {

            sendblockreadrequest(blocksize, table, blocksize);

            ext2_write32(&entries[index], block);

            sendblockwriterequest(blocksize, table, blocksize);

        }

        return block;

    }

    return ext2_read32(&entries[index]);

}

static unsigned int allocsector(struct ext2_node *node, unsigned int offset, unsigned int id)
{

    unsigned int igroup = (id - 1) / sb.nodeCountGroup;
    unsigned int index = offset / blocksize;
    unsigned int slots = blocksize / 4;
    unsigned int depth;
    unsigned int table;

    if (node->flags & EXT2_NODE_EXTENTS)
        return allocextent(node, index, id);

    if (index < 12)
    {

        unsigned int sector = allocblock(igroup);

        if (sector)
        {

            node->pointer[index] = sector;
            node->sectorCount += blocksize / 512;

        }

        return sector;

    }

    index -= 12;

    if (index < slots)
    {

        if (!node->singlyIndirectPointer)
            node->singlyIndirectPointer = alloctable(node, igroup);

        table = node->singlyIndirectPointer;
        depth = 1;

    }

    else if (index - slots < slots * slots)
    {

        if (!node->doublyIndirectPointer)
            node->doublyIndirectPointer = alloctable(node, igroup);

        index -= slots;
        table = node->doublyIndirectPointer;
        depth = 2;

    }

    else
    {

        if (!node->tripplyIndirectPointer)
            node->tripplyIndirectPointer = alloctable(node, igroup);

        index -= slots + slots * slots;
        table = node->tripplyIndirectPointer;
        depth = 3;

    }

    for (; table && depth; depth--)
    {

        unsigned int divisor = 1;
        unsigned int i;

        for (i = 1; i < depth; i++)
            divisor *= slots;

        table = allocentry(node, table, (index / divisor) % slots, depth > 1, igroup);

    }

    return table;

}

static unsigned int getrecord(struct ext2_entry *entry, struct record *record, unsigned int offset)
{

    switch (entry->type)
    {

    case 1:
        record_init(record, ext2_read32(&entry->node), RECORD_TYPE_NORMAL, 0, offset + ext2_read16(&entry->size), entry->length, (char *)(entry + 1));

        return 1;

    case 2:
        record_init(record, ext2_read32(&entry->node), RECORD_TYPE_DIRECTORY, 0, offset + ext2_read16(&entry->size), entry->length, (char *)(entry + 1));

        return 1;

    }

    return 0;

}

static struct ext2_entry *getentry(struct ext2_node *node, unsigned int offset)
{

    unsigned int sector = getsector(node, offset);

    if (sector)
    {

        struct ext2_entry *entry = (struct ext2_entry *)((char *)blockinfo.buffer + (offset % blocksize));

        sendblockreadrequest(blocksize, sector, blocksize);

        if (ext2_read16(&entry->size))
            return entry;

    }

    return 0;

}

static unsigned int matchentry(struct ext2_node *node, char *name, unsigned int length)
{

    unsigned int offset = 0;

    while (offset < node->sizeLow)
    {

        struct ext2_entry *entry = getentry(node, offset);

        if (!entry)
            break;

        if (ext2_read32(&entry->node) && entry->length == length && buffer_match(entry + 1, name, length))
            return ext2_read32(&entry->node);

        offset += ext2_read16(&entry->size);

    }

    return 0;

}

static void writeentry(struct ext2_entry *entry, unsigned int node, unsigned int size, unsigned int length, unsigned int type, char *name)
{

    ext2_write32(&entry->node, node);
    ext2_write16(&entry->size, size);
    entry->length = length;
    entry->type = type;

    buffer_copy(entry + 1, name, length);

}

static unsigned int entrysize(unsigned int namelength)
{

    return (8 + namelength + 3) & ~3;

}

static unsigned int istail(struct ext2_entry *entry)
{

    return !ext2_read32(&entry->node) && ext2_read16(&entry->size) == EXT2_ENTRY_TAILSIZE && !entry->length && entry->type == EXT2_ENTRY_TAILTYPE;

}

static unsigned int hastail(char *block)
{

    return !hasmetadatachecksums() || istail((struct ext2_entry *)(block + blocksize - EXT2_ENTRY_TAILSIZE));

}

static void setentrychecksum(char *block, struct ext2_node *node, unsigned int id)
{

    if (hasmetadatachecksums())
        ext2_write32(block + blocksize - 4, ext2_crc32c(getnodeseed(id, node->generation), block, blocksize - EXT2_ENTRY_TAILSIZE));

}

static unsigned int spliceentry(struct ext2_node *node, unsigned int parentid, unsigned int id, unsigned int length, unsigned int type, char *name)
{

    unsigned int realsize = entrysize(length);
    unsigned int offset = 0;

    while (offset < node->sizeLow)
    {

        unsigned int sector = getsector(node, offset);
        struct ext2_entry *entry = getentry(node, offset);
        unsigned int used;
        unsigned int size;

        if (!entry)
            break;

        used = ext2_read32(&entry->node) ? entrysize(entry->length) : 0;
        size = ext2_read16(&entry->size) - used;

        if (size >= realsize && !(hasmetadatachecksums() && istail(entry)) && hastail((char *)blockinfo.buffer))
        {

            if (used)
                ext2_write16(&entry->size, used);

            writeentry((struct ext2_entry *)((char *)entry + used), id, size, length, type, name);
            setentrychecksum((char *)blockinfo.buffer, node, parentid);
            sendblockwriterequest(blocksize, sector, blocksize);

            return 1;

        }

        offset += ext2_read16(&entry->size);

    }

    return 0;

}

static unsigned int growentry(struct ext2_node *node, unsigned int parentid, unsigned int id, unsigned int length, unsigned int type, char *name)
{

    unsigned int sector = allocsector(node, node->sizeLow, parentid);

    if (sector)
    {

        char *block = (char *)blockinfo.buffer;

        buffer_clear(block, EXT2_MAXBLOCKSIZE);

        if (hasmetadatachecksums())
        {

            writeentry((struct ext2_entry *)block, id, blocksize - EXT2_ENTRY_TAILSIZE, length, type, name);
            writeentry((struct ext2_entry *)(block + blocksize - EXT2_ENTRY_TAILSIZE), 0, EXT2_ENTRY_TAILSIZE, 0, EXT2_ENTRY_TAILTYPE, "");
            setentrychecksum(block, node, parentid);

        }

        else
        {

            writeentry((struct ext2_entry *)block, id, blocksize, length, type, name);

        }

        sendblockwriterequest(blocksize, sector, blocksize);

        node->sizeLow += blocksize;

        return 1;

    }

    return 0;

}

static unsigned int addentry(struct ext2_node *node, unsigned int parentid, unsigned int id, unsigned int length, unsigned int type, char *name)
{

    if (spliceentry(node, parentid, id, length, type, name))
        return 1;

    return growentry(node, parentid, id, length, type, name);

}

static unsigned int removeentry(struct ext2_node *parent, unsigned int parentid, unsigned int id)
{

    unsigned int offset;

    for (offset = 0; offset < parent->sizeLow; offset += blocksize)
    {

        unsigned int sector = getsector(parent, offset);
        char *block = (char *)blockinfo.buffer;
        unsigned int position = 0;
        unsigned int previous = blocksize;

        if (!sector)
            return 0;

        sendblockreadrequest(blocksize, sector, blocksize);

        while (position < blocksize)
        {

            struct ext2_entry *entry = (struct ext2_entry *)(block + position);

            if (!ext2_read16(&entry->size))
                break;

            if (ext2_read32(&entry->node) == id)
            {

                struct ext2_entry *before = (struct ext2_entry *)(block + previous);

                if (!hastail(block))
                    return 0;

                if (previous == blocksize)
                    ext2_write32(&entry->node, 0);
                else
                    ext2_write16(&before->size, ext2_read16(&before->size) + ext2_read16(&entry->size));

                setentrychecksum(block, parent, parentid);
                sendblockwriterequest(blocksize, sector, blocksize);

                return 1;

            }

            previous = position;
            position += ext2_read16(&entry->size);

        }

    }

    return 0;

}

static unsigned int isempty(struct ext2_node *node)
{

    unsigned int offset = 0;

    while (offset < node->sizeLow)
    {

        struct ext2_entry *entry = getentry(node, offset);
        char *name;

        if (!entry)
            return 0;

        name = (char *)(entry + 1);

        if (ext2_read32(&entry->node) && !(entry->length == 1 && name[0] == '.') && !(entry->length == 2 && name[0] == '.' && name[1] == '.'))
            return 0;

        offset += ext2_read16(&entry->size);

    }

    return 1;

}

static unsigned int readdirectory(struct ext2_node *node, unsigned int roffset, unsigned int rcount, unsigned int capacity, void *data)
{

    struct record *records = data;
    unsigned int count = 0;
    unsigned int i = 0;

    while (roffset < node->sizeLow && (i + 1) * sizeof (struct record) <= capacity)
    {

        struct ext2_entry *entry = getentry(node, roffset);

        if (!entry)
            break;

        if (ext2_read32(&entry->node) && getrecord(entry, &records[i], roffset))
        {

            i++;
            count += sizeof (struct record);

        }

        roffset += ext2_read16(&entry->size);

    }

    return count;

}

static unsigned int writedirectory(struct ext2_node *node, unsigned int roffset, unsigned int rcount, unsigned int rid, void *data)
{

    return 0;

}

static unsigned int getfilesize(struct ext2_node *node)
{

    return (node->sizeHigh) ? 0xFFFFFFFF : node->sizeLow;

}

static struct span getspan(struct ext2_node *node, unsigned int roffset, unsigned int rcount, unsigned int capacity)
{

    unsigned int remaining = getfilesize(node) - roffset;
    unsigned int blockoffset = roffset % blocksize;
    struct span span;
    struct run run;

    span.count = capacity;

    if (span.count > rcount)
        span.count = rcount;

    if (span.count > remaining)
        span.count = remaining;

    if (span.count > blockinfo.buffersize - blockoffset)
        span.count = blockinfo.buffersize - blockoffset;

    run = getrun(node, roffset, (blockoffset + span.count + blocksize - 1) / blocksize);
    span.sector = run.sector;
    span.blocks = run.blocks;

    if (span.count > span.blocks * blocksize - blockoffset)
        span.count = span.blocks * blocksize - blockoffset;

    return span;

}

static unsigned int readfile(struct ext2_node *node, unsigned int roffset, unsigned int rcount, unsigned int capacity, void *data)
{

    if (roffset < getfilesize(node))
    {

        struct span span = getspan(node, roffset, rcount, capacity);

        if (span.sector)
        {

            sendblockreadrequest(span.blocks * blocksize, span.sector, blocksize);

            return buffer_write(data, capacity, (char *)blockinfo.buffer + roffset % blocksize, span.count, 0);

        }

        buffer_clear(data, span.count);

        return span.count;

    }

    return 0;

}

static unsigned int sendfile(struct ext2_node *node, unsigned int roffset, unsigned int rcount, unsigned int target)
{

    if (roffset < getfilesize(node))
    {

        struct span span = getspan(node, roffset, rcount, rcount);
        unsigned int count = span.blocks * blocksize;

        if (span.sector && sendblockrequest(MESSAGE_TYPE_BLOCKREADREQUEST, MESSAGE_TYPE_BLOCKREADRESPONSE, count, span.sector, blocksize, target) == count)
            return span.count;

    }

    return 0;

}

static void cleartail(struct ext2_node *node, unsigned int roffset)
{

    unsigned int blockoffset = node->sizeLow % blocksize;
    unsigned int sector = getsector(node, node->sizeLow);
    unsigned int count = blocksize - blockoffset;

    if (!blockoffset || !sector)
        return;

    if (count > roffset - node->sizeLow)
        count = roffset - node->sizeLow;

    sendblockreadrequest(blocksize, sector, blocksize);
    buffer_clear((char *)blockinfo.buffer + blockoffset, count);
    sendblockwriterequest(blocksize, sector, blocksize);

}

static unsigned int writefile(struct ext2_node *node, unsigned int roffset, unsigned int rcount, unsigned int rid, void *data)
{

    unsigned int changed = 0;
    unsigned int total = 0;

    if (node->sizeHigh)
        return 0;

    if (roffset > node->sizeLow)
        cleartail(node, roffset);

    while (total < rcount)
    {

        unsigned int offset = roffset + total;
        unsigned int blockoffset = offset % blocksize;
        unsigned int sector = getsector(node, offset);
        unsigned int count = rcount - total;

        if (count > blocksize - blockoffset)
            count = blocksize - blockoffset;

        if (sector)
        {

            if (count < blocksize)
                sendblockreadrequest(blocksize, sector, blocksize);

        }

        else
        {

            sector = allocsector(node, offset, rid);
            changed = 1;

            if (!sector)
                break;

            buffer_clear((void *)blockinfo.buffer, blocksize);

        }

        buffer_copy((char *)blockinfo.buffer + blockoffset, (char *)data + total, count);
        sendblockwriterequest(blocksize, sector, blocksize);

        total += count;

    }

    if (roffset + total > node->sizeLow)
    {

        node->sizeLow = roffset + total;
        changed = 1;

    }

    if (changed)
        simplewrite(node, rid);

    return total;

}

static unsigned int stat(unsigned int id, struct record *record)
{

    if (id)
    {

        struct ext2_node node;

        simpleread(&node, id);
        record_init(record, id, ((node.type & 0xF000) == 0x4000) ? RECORD_TYPE_DIRECTORY : RECORD_TYPE_NORMAL, ((node.type & 0xF000) == 0x8000) ? getfilesize(&node) : node.sizeLow, 0, 0, "");

        return sizeof (struct record);

    }

    return 0;

}

static unsigned int walk(unsigned int id, char *path, unsigned int length)
{

    unsigned int offset = 0;

    while (offset < length)
    {

        unsigned int next = buffer_eachbyte(path, length, '/', offset);
        unsigned int count = (next ? next : length + 1) - offset - 1;

        if (count)
        {

            struct ext2_node node;

            simpleread(&node, id);

            switch (node.type & 0xF000)
            {

            case 0x4000:
                id = matchentry(&node, path + offset, count);

                break;

            default:
                id = 0;

                break;

            }

            if (!id)
                return 0;

        }

        offset += count + 1;

    }

    return id;

}

static void oncreaterequest(struct message *message)
{

    struct message_createrequest *request = message_validate(MESSAGE_TYPE_CREATEREQUEST, message->data, message->length);
    struct message_createresponse response;
    struct ext2_node parent;

    response.id = 0;

    if (request && mode == EXT2_MOUNT_WRITE)
    {

        simpleread(&parent, request->parent);

        switch (parent.type & 0xF000)
        {

        case 0x4000:
            {

                unsigned int igroup = (request->parent - 1) / sb.nodeCountGroup;
                unsigned int id = matchentry(&parent, (char *)(request + 1), request->count);

                if (id)
                {

                    response.id = id;

                    break;

                }

                if (parent.flags & EXT2_NODE_INDEX)
                    break;

                id = allocnode(igroup);

                if (id)
                {

                    struct ext2_node node;

                    buffer_clear(&node, sizeof (struct ext2_node));

                    node.type = 0x81A4;
                    node.hardCount = 1;

                    simplecreate(&node, id);

                    if (addentry(&parent, request->parent, id, request->count, 1, (char *)(request + 1)))
                    {

                        simplewrite(&parent, request->parent);

                        response.id = id;

                    }

                    else
                    {

                        buffer_clear(&node, sizeof (struct ext2_node));
                        simplewrite(&node, id);
                        freenode(id, 0);

                    }

                }

            }

            break;

        }

    }

    channel_send(0, message->source, MESSAGE_TYPE_CREATERESPONSE, sizeof (struct message_createresponse), &response);

}

static void onreadrequest(struct message *message)
{

    unsigned char data[MESSAGE_SIZE];
    struct message_readrequest *request = message_validate(MESSAGE_TYPE_READREQUEST, message->data, message->length);
    struct message_readresponse *response = (struct message_readresponse *)data;
    struct ext2_node node;

    response->count = 0;
    response->buffer = 0;

    if (request)
    {

        simpleread(&node, request->id);

        switch (node.type & 0xF000)
        {

        case 0x4000:
            response->count = readdirectory(&node, request->offset, request->count, MESSAGE_SIZE - sizeof (struct message_readresponse), response + 1);

            break;

        case 0x8000:
            if (request->flags & READ_FLAG_BUFFER)
                response->count = sendfile(&node, request->offset, request->count, message->source);

            if (response->count)
                response->buffer = blockinfo.buffer + request->offset % blocksize;
            else
                response->count = readfile(&node, request->offset, request->count, MESSAGE_SIZE - sizeof (struct message_readresponse), response + 1);

            break;

        }

    }

    channel_send(0, message->source, MESSAGE_TYPE_READRESPONSE, sizeof (struct message_readresponse) + ((response->buffer) ? 0 : response->count), data);

}

static void freeindirect(unsigned int block, unsigned int depth)
{

    unsigned int table[EXT2_MAXBLOCKSIZE / 4];
    unsigned int i;

    sendblockreadrequest(blocksize, block, blocksize);
    buffer_copy(table, (void *)blockinfo.buffer, blocksize);

    for (i = 0; i < blocksize / 4; i++)
    {

        unsigned int entry = ext2_read32(&table[i]);

        if (!entry)
            continue;

        if (depth > 1)
            freeindirect(entry, depth - 1);
        else
            freeblock(entry);

    }

    freeblock(block);

}

static void freeextents(struct ext2_extentheader *header, unsigned int size, unsigned int level)
{

    unsigned int entries = ext2_read16(&header->entries);
    unsigned int i;

    if (!isextentheader(header, size) || level > EXT2_EXTENTMAXDEPTH)
        return;

    if (ext2_read16(&header->depth))
    {

        struct ext2_extentindex *entry = (struct ext2_extentindex *)(header + 1);

        for (i = 0; i < entries; i++, entry++)
        {

            unsigned int leaf = ext2_read32(&entry->leaf);
            char data[EXT2_MAXBLOCKSIZE];

            if (ext2_read16(&entry->leafHigh) || sendblockreadrequest(blocksize, leaf, blocksize) != blocksize)
                continue;

            buffer_copy(data, (void *)blockinfo.buffer, blocksize);
            freeextents((struct ext2_extentheader *)data, blocksize, level + 1);
            freeblock(leaf);

        }

    }

    else
    {

        struct ext2_extent *extent = (struct ext2_extent *)(header + 1);

        for (i = 0; i < entries; i++, extent++)
        {

            unsigned int start = ext2_read32(&extent->start);
            unsigned int length = getextentlength(extent);
            unsigned int j;

            if (ext2_read16(&extent->startHigh))
                continue;

            for (j = 0; j < length; j++)
                freeblock(start + j);

        }

    }

}

static void freeattributes(unsigned int block)
{

    char *data = readmeta(block);
    unsigned int refcount = ext2_read32(data + EXT2_ATTRIBUTE_REFCOUNT);

    if (ext2_read32(data) != EXT2_ATTRIBUTEMAGIC)
        return;

    if (refcount <= 1)
    {

        freeblock(block);

        return;

    }

    buffer_copy((void *)blockinfo.buffer, data, blocksize);
    ext2_write32((char *)blockinfo.buffer + EXT2_ATTRIBUTE_REFCOUNT, refcount - 1);

    if (hasmetadatachecksums())
    {

        unsigned char number[8];

        ext2_write32(number, block);
        ext2_write32(number + 4, 0);
        ext2_write32((char *)blockinfo.buffer + EXT2_ATTRIBUTE_CHECKSUM, 0);
        ext2_write32((char *)blockinfo.buffer + EXT2_ATTRIBUTE_CHECKSUM, ext2_crc32c(ext2_crc32c(csumseed, number, 8), (void *)blockinfo.buffer, blocksize));

    }

    sendblockwriterequest(blocksize, block, blocksize);

}

static unsigned int removenode(unsigned int parentid, unsigned int id)
{

    struct ext2_node parent;
    struct ext2_node node;
    unsigned int directory;
    unsigned int i;

    if (id == 2 || id == parentid)
        return 0;

    simpleread(&parent, parentid);
    simpleread(&node, id);

    if ((parent.type & 0xF000) != 0x4000)
        return 0;

    switch (node.type & 0xF000)
    {

    case 0x4000:
        if (!isempty(&node))
            return 0;

        directory = 1;

        break;

    case 0x8000:
        directory = 0;

        break;

    default:
        return 0;

    }

    if (!removeentry(&parent, parentid, id))
        return 0;

    if (directory)
    {

        parent.hardCount--;

        simplewrite(&parent, parentid);

    }

    else if (node.hardCount > 1)
    {

        node.hardCount--;

        simplewrite(&node, id);

        return 1;

    }

    if (node.flags & EXT2_NODE_EXTENTS)
    {

        struct extentroot root;

        getextentroot(&node, &root);
        freeextents((struct ext2_extentheader *)root.data, EXTENTROOTSIZE, 0);

    }

    else
    {

        for (i = 0; i < 12; i++)
        {

            if (node.pointer[i])
                freeblock(node.pointer[i]);

        }

        if (node.singlyIndirectPointer)
            freeindirect(node.singlyIndirectPointer, 1);

        if (node.doublyIndirectPointer)
            freeindirect(node.doublyIndirectPointer, 2);

        if (node.tripplyIndirectPointer)
            freeindirect(node.tripplyIndirectPointer, 3);

    }

    if (node.extendedAttributes)
        freeattributes(node.extendedAttributes);

    buffer_clear(&node, sizeof (struct ext2_node));
    simplewrite(&node, id);
    freenode(id, directory);

    return 1;

}

static void onremoverequest(struct message *message)
{

    struct message_removerequest *request = message_validate(MESSAGE_TYPE_REMOVEREQUEST, message->data, message->length);
    struct message_removeresponse response;

    response.status = (request && mode == EXT2_MOUNT_WRITE) ? removenode(request->parent, request->id) : 0;

    channel_send(0, message->source, MESSAGE_TYPE_REMOVERESPONSE, sizeof (struct message_removeresponse), &response);

}

static void onstatrequest(struct message *message)
{

    unsigned char data[MESSAGE_SIZE];
    struct message_statrequest *request = message_validate(MESSAGE_TYPE_STATREQUEST, message->data, message->length);
    struct message_statresponse *response = (struct message_statresponse *)data;

    response->count = (request) ? stat(request->id, (struct record *)(response + 1)) : 0;

    channel_send(0, message->source, MESSAGE_TYPE_STATRESPONSE, sizeof (struct message_statresponse) + response->count, data);

}

static void onwalkrequest(struct message *message)
{

    struct message_walkrequest *request = message_validate(MESSAGE_TYPE_WALKREQUEST, message->data, message->length);
    struct message_walkresponse response;

    response.id = (request) ? walk((request->parent) ? request->parent : 2, (char *)(request + 1), request->length) : 0;

    channel_send(0, message->source, MESSAGE_TYPE_WALKRESPONSE, sizeof (struct message_walkresponse), &response);

}

static void onwriterequest(struct message *message)
{

    struct message_writerequest *request = message_validate(MESSAGE_TYPE_WRITEREQUEST, message->data, message->length);
    struct message_writeresponse response;
    struct ext2_node node;

    response.count = 0;

    if (request && mode == EXT2_MOUNT_WRITE)
    {

        simpleread(&node, request->id);

        switch (node.type & 0xF000)
        {

        case 0x4000:
            response.count = writedirectory(&node, request->offset, request->count, request->id, request + 1);

            break;

        case 0x8000:
            response.count = writefile(&node, request->offset, request->count, request->id, request + 1);

            break;

        }

    }

    channel_send(0, message->source, MESSAGE_TYPE_WRITERESPONSE, sizeof (struct message_writeresponse), &response);

}

static void onmain(struct message *message)
{

    block = channel_lookup(option_getstring("block-service"));
    partsector = option_getdecimal("partsector");

    if (block)
    {

        channel_send(1, block, MESSAGE_TYPE_INFO, 0, 0);

        if (!channel_wait(1, block, MESSAGE_TYPE_BLOCKINFO, sizeof (struct message_blockinfo), &blockinfo))
            return;

        readsuperblock();

        mode = ext2_validate((struct ext2_superblock *)blockinfo.buffer);

        if (mode)
        {

            if (sb.blockCount > (0xFFFFFFFF - partsector) / (blocksize / MESSAGE_SECTORSIZE) || ((sb.requiredFeatures & EXT2_INCOMPAT_64BIT) && sb.blockCountHigh))
            {

                channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "Volume too large: %s\n", option_getstring("service"));

                return;

            }

            if (call_announce(0, cstring_length(option_getstring("service")), option_getstring("service")) != MESSAGE_OK)
            {

                channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "Could not announce service: %s\n", option_getstring("service"));

                return;

            }

            channel_hold(0);

        }

    }

}

void init(void)
{

    option_add("service", "ext2");
    option_add("block-service", "block:0");
    option_add("partsector", "0");
    channel_bind(MESSAGE_TYPE_MAIN, onmain);
    channel_bind(MESSAGE_TYPE_CREATEREQUEST, oncreaterequest);
    channel_bind(MESSAGE_TYPE_READREQUEST, onreadrequest);
    channel_bind(MESSAGE_TYPE_REMOVEREQUEST, onremoverequest);
    channel_bind(MESSAGE_TYPE_STATREQUEST, onstatrequest);
    channel_bind(MESSAGE_TYPE_WALKREQUEST, onwalkrequest);
    channel_bind(MESSAGE_TYPE_WRITEREQUEST, onwriterequest);

}

