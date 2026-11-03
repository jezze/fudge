#include <fudge.h>
#include <abi.h>
#include <disk.h>

static struct message_blockinfo blockinfo;

static unsigned int block;
static unsigned int partoffset;

#define CACHESLOTS 4

struct cacheslot
{

    unsigned int offset;
    unsigned int count;
    char data[EXT2_MAXBLOCKSIZE];

};

struct span
{

    unsigned int sector;
    unsigned int blocks;
    unsigned int count;

};

static struct cacheslot cache[CACHESLOTS];
static unsigned int cachenext;

static void updatecache(unsigned int offset, unsigned int count)
{

    unsigned int i;

    for (i = 0; i < CACHESLOTS; i++)
    {

        struct cacheslot *slot = &cache[i];

        if (slot->offset + slot->count <= offset || slot->offset >= offset + count)
            continue;

        if (slot->offset >= offset && slot->offset + slot->count <= offset + count)
            buffer_copy(slot->data, (char *)blockinfo.buffer + (slot->offset - offset), slot->count);
        else
            slot->count = 0;

    }

}

static unsigned int sendblockrequest(unsigned int event, unsigned int reply, unsigned int count, unsigned int sector, unsigned int blocksize, unsigned int target)
{

    struct message_blockrequest request;
    struct message_blockresponse response;

    request.offset = partoffset + sector * blocksize;
    request.count = count;
    request.target = target;

    if (!channel_send(1, block, event, sizeof (struct message_blockrequest), &request))
        return 0;

    return (channel_wait(1, block, reply, sizeof (struct message_blockresponse), &response)) ? response.count : 0;

}

static unsigned int sendblockreadrequest(unsigned int count, unsigned int sector, unsigned int blocksize)
{

    return sendblockrequest(MESSAGE_BLOCKREADREQUEST, MESSAGE_BLOCKREADRESPONSE, count, sector, blocksize, 0);

}

static unsigned int sendblockwriterequest(unsigned int count, unsigned int sector, unsigned int blocksize)
{

    unsigned int written = sendblockrequest(MESSAGE_BLOCKWRITEREQUEST, MESSAGE_BLOCKWRITERESPONSE, count, sector, blocksize, 0);

    updatecache(sector * blocksize, written);

    return written;

}

static struct ext2_superblock sb;
static unsigned int blocksize;

static char *readmeta(unsigned int sector)
{

    unsigned int offset = sector * blocksize;
    struct cacheslot *slot;
    unsigned int i;

    for (i = 0; i < CACHESLOTS; i++)
    {

        slot = &cache[i];

        if (slot->count == blocksize && slot->offset == offset)
            return slot->data;

    }

    if (sendblockreadrequest(blocksize, sector, blocksize) != blocksize || blocksize > EXT2_MAXBLOCKSIZE)
        return (char *)blockinfo.buffer;

    slot = &cache[cachenext];
    cachenext = (cachenext + 1) % CACHESLOTS;

    buffer_copy(slot->data, (void *)blockinfo.buffer, blocksize);

    slot->offset = offset;
    slot->count = blocksize;

    return slot->data;

}

static void readsuperblock(void)
{

    sendblockreadrequest(1024, 1, 1024);
    buffer_copy(&sb, (void *)blockinfo.buffer, sizeof (struct ext2_superblock));
    ext2_convertsuperblock(&sb);

    blocksize = (1024 << sb.blockSize);

}

static void writesuperblock(void)
{

    sendblockreadrequest(1024, 1, 1024);
    buffer_copy((void *)blockinfo.buffer, &sb, sizeof (struct ext2_superblock));
    ext2_convertsuperblock((struct ext2_superblock *)blockinfo.buffer);
    sendblockwriterequest(1024, 1, 1024);

}

static void readblockgroup(struct ext2_blockgroup *bg, unsigned int index)
{

    unsigned int slots = blocksize / sizeof (struct ext2_blockgroup);
    unsigned int sector = sb.superblockIndex + 1 + index / slots;
    unsigned int offset = (index % slots) * sizeof (struct ext2_blockgroup);

    buffer_copy(bg, readmeta(sector) + offset, sizeof (struct ext2_blockgroup));
    ext2_convertblockgroup(bg);

}

static void writeblockgroup(struct ext2_blockgroup *bg, unsigned int index)
{

    unsigned int slots = blocksize / sizeof (struct ext2_blockgroup);
    unsigned int sector = sb.superblockIndex + 1 + index / slots;
    unsigned int offset = (index % slots) * sizeof (struct ext2_blockgroup);

    sendblockreadrequest(blocksize, sector, blocksize);
    buffer_copy((char *)blockinfo.buffer + offset, bg, sizeof (struct ext2_blockgroup));
    ext2_convertblockgroup((struct ext2_blockgroup *)((char *)blockinfo.buffer + offset));
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

static void writenode(struct ext2_node *node, unsigned int start, unsigned int index)
{

    unsigned int slots = blocksize / sb.nodeSize;
    unsigned int sector = start + index / slots;
    unsigned int offset = (index % slots) * sb.nodeSize;

    sendblockreadrequest(blocksize, sector, blocksize);
    buffer_copy((char *)blockinfo.buffer + offset, node, sizeof (struct ext2_node));
    ext2_convertnode((struct ext2_node *)((char *)blockinfo.buffer + offset));
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
    writenode(node, bg.blockTableAddress, inode);

}

static unsigned int allocblockgroup(unsigned int blockgroup)
{

    struct ext2_blockgroup bg;
    unsigned char *bitmap = (unsigned char *)blockinfo.buffer;
    unsigned int i;

    readblockgroup(&bg, blockgroup);

    if (!bg.blockCountUnalloc)
        return 0;

    sendblockreadrequest(blocksize, bg.blockUsageAddress, blocksize);

    for (i = 0; i < sb.blockCountGroup; i++)
    {

        unsigned int index = i / 8;
        unsigned int bit = 1 << (i % 8);

        if (!(bitmap[index] & bit))
        {

            bitmap[index] |= bit;

            sendblockwriterequest(blocksize, bg.blockUsageAddress, blocksize);

            bg.blockCountUnalloc--;

            writeblockgroup(&bg, blockgroup);

            sb.blockCountUnalloc--;

            writesuperblock();

            return sb.superblockIndex + blockgroup * sb.blockCountGroup + i;

        }

    }

    return 0;

}

static unsigned int allocnodegroup(unsigned int igroup)
{

    struct ext2_blockgroup bg;
    unsigned char *bitmap = (unsigned char *)blockinfo.buffer;
    unsigned int i;

    readblockgroup(&bg, igroup);

    if (!bg.nodeCountUnalloc)
        return 0;

    sendblockreadrequest(blocksize, bg.nodeUsageAddress, blocksize);

    for (i = 0; i < sb.nodeCountGroup; i++)
    {

        unsigned int index = i / 8;
        unsigned int bit = 1 << (i % 8);

        if (!(bitmap[index] & bit))
        {

            bitmap[index] |= bit;

            sendblockwriterequest(blocksize, bg.nodeUsageAddress, blocksize);

            bg.nodeCountUnalloc--;

            writeblockgroup(&bg, igroup);

            sb.nodeCountUnalloc--;

            writesuperblock();

            return igroup * sb.nodeCountGroup + i + 1;

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

static unsigned int getrun(struct ext2_node *node, unsigned int offset, unsigned int max, unsigned int *count)
{

    unsigned int index = offset / blocksize;
    unsigned int slots = blocksize / 4;
    unsigned int direct = index < 12;
    unsigned int *entries;
    unsigned int available;
    unsigned int table;
    unsigned int first;

    if (direct)
    {

        entries = node->pointer + index;
        available = 12 - index;

    }

    else
    {

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

        *count = 1;

        if (!table)
            return 0;

        entries = (unsigned int *)readmeta(table) + index % slots;
        available = slots - index % slots;

    }

    if (available > max)
        available = max;

    first = (direct) ? entries[0] : ext2_read32(entries);

    for (*count = 1; *count < available; (*count)++)
    {

        unsigned int next = (direct) ? entries[*count] : ext2_read32(&entries[*count]);

        if (next != ((first) ? first + *count : 0))
            break;

    }

    return first;

}

static unsigned int getsector(struct ext2_node *node, unsigned int offset)
{

    unsigned int count;

    return getrun(node, offset, 1, &count);

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
    unsigned int block;

    sendblockreadrequest(blocksize, table, blocksize);

    if (ext2_read32(&entries[index]))
        return ext2_read32(&entries[index]);

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

static unsigned int allocsector(struct ext2_node *node, unsigned int offset, unsigned int igroup)
{

    unsigned int index = offset / blocksize;
    unsigned int slots = blocksize / 4;
    unsigned int depth;
    unsigned int table;

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

static void getrecord(struct ext2_entry *entry, struct record *record, unsigned int offset)
{

    switch (entry->type)
    {

    case 1:
        record_init(record, ext2_read32(&entry->node), RECORD_TYPE_NORMAL, 0, offset + ext2_read16(&entry->size), entry->length, (char *)(entry + 1));

        break;

    case 2:
        record_init(record, ext2_read32(&entry->node), RECORD_TYPE_DIRECTORY, 0, offset + ext2_read16(&entry->size), entry->length, (char *)(entry + 1));

        break;

    }

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

static unsigned int spliceentry(struct ext2_node *node, unsigned int id, unsigned int length, unsigned int type, char *name)
{

    unsigned int realsize = entrysize(length);
    unsigned int offset = 0;

    while (offset < node->sizeLow)
    {

        struct ext2_entry *entry = getentry(node, offset);
        unsigned int used;
        unsigned int size;

        if (!entry)
            break;

        used = ext2_read32(&entry->node) ? entrysize(entry->length) : 0;
        size = ext2_read16(&entry->size) - used;

        if (size >= realsize)
        {

            unsigned int sector = getsector(node, offset);

            if (used)
                ext2_write16(&entry->size, used);

            writeentry((struct ext2_entry *)((char *)entry + used), id, size, length, type, name);
            sendblockwriterequest(blocksize, sector, blocksize);

            return 1;

        }

        offset += ext2_read16(&entry->size);

    }

    return 0;

}

static unsigned int growentry(struct ext2_node *node, unsigned int igroup, unsigned int id, unsigned int length, unsigned int type, char *name)
{

    unsigned int sector = allocsector(node, node->sizeLow, igroup);

    if (sector)
    {

        buffer_clear((void *)blockinfo.buffer, EXT2_MAXBLOCKSIZE);
        writeentry((struct ext2_entry *)blockinfo.buffer, id, blocksize, length, type, name);
        sendblockwriterequest(blocksize, sector, blocksize);

        node->sizeLow += blocksize;

        return 1;

    }

    return 0;

}

static unsigned int addentry(struct ext2_node *node, unsigned int igroup, unsigned int id, unsigned int length, unsigned int type, char *name)
{

    if (spliceentry(node, id, length, type, name))
        return 1;

    return growentry(node, igroup, id, length, type, name);

}

static unsigned int removeentry(struct ext2_node *parent, unsigned int id)
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

                if (previous == blocksize)
                    ext2_write32(&entry->node, 0);
                else
                    ext2_write16(&before->size, ext2_read16(&before->size) + ext2_read16(&entry->size));

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

        if (ext2_read32(&entry->node))
        {

            getrecord(entry, &records[i], roffset);

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

static struct span getspan(struct ext2_node *node, unsigned int roffset, unsigned int rcount, unsigned int capacity)
{

    unsigned int remaining = node->sizeLow - roffset;
    unsigned int blockoffset = roffset % blocksize;
    struct span span;

    span.count = capacity;

    if (span.count > rcount)
        span.count = rcount;

    if (span.count > remaining)
        span.count = remaining;

    if (span.count > blockinfo.buffersize - blockoffset)
        span.count = blockinfo.buffersize - blockoffset;

    span.sector = getrun(node, roffset, (blockoffset + span.count + blocksize - 1) / blocksize, &span.blocks);

    if (span.count > span.blocks * blocksize - blockoffset)
        span.count = span.blocks * blocksize - blockoffset;

    return span;

}

static unsigned int readfile(struct ext2_node *node, unsigned int roffset, unsigned int rcount, unsigned int capacity, void *data)
{

    if (roffset < node->sizeLow)
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

/* the blocks go straight into the target's block buffer; holes and failures return 0 and are read the usual way */
static unsigned int sendfile(struct ext2_node *node, unsigned int roffset, unsigned int rcount, unsigned int target)
{

    if (roffset < node->sizeLow)
    {

        struct span span = getspan(node, roffset, rcount, rcount);
        unsigned int count = span.blocks * blocksize;

        if (span.sector && sendblockrequest(MESSAGE_BLOCKREADREQUEST, MESSAGE_BLOCKREADRESPONSE, count, span.sector, blocksize, target) == count)
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

    unsigned int igroup = (rid - 1) / sb.nodeCountGroup;
    unsigned int changed = 0;
    unsigned int total = 0;

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

            sector = allocsector(node, offset, igroup);
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

    struct ext2_node node;

    if (!id)
        return 0;

    simpleread(&node, id);
    record_init(record, id, ((node.type & 0xF000) == 0x4000) ? RECORD_TYPE_DIRECTORY : RECORD_TYPE_NORMAL, node.sizeLow, 0, 0, "");

    return sizeof (struct record);

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

/* the request's header and the count bytes after it are all in the message */
static void oncreaterequest(struct message *message)
{

    struct message_createrequest *request = message_validate(MESSAGE_CREATEREQUEST, message->data, message->length);
    struct message_createresponse response;
    struct ext2_node parent;

    response.id = 0;

    if (request)
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

                id = allocnode(igroup);

                if (id)
                {

                    struct ext2_node node;

                    buffer_clear(&node, sizeof (struct ext2_node));

                    node.type = 0x81A4;
                    node.hardCount = 1;

                    simplewrite(&node, id);

                    if (addentry(&parent, igroup, id, request->count, 1, (char *)(request + 1)))
                    {

                        simplewrite(&parent, request->parent);

                        response.id = id;

                    }

                }

            }

            break;

        }

    }

    channel_send(0, message->source, MESSAGE_CREATERESPONSE, sizeof (struct message_createresponse), &response);

}

static void onreadrequest(struct message *message)
{

    unsigned char data[MESSAGE_SIZE];
    struct message_readrequest *request = message_validate(MESSAGE_READREQUEST, message->data, message->length);
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

    channel_send(0, message->source, MESSAGE_READRESPONSE, sizeof (struct message_readresponse) + ((response->buffer) ? 0 : response->count), data);

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

    if (!removeentry(&parent, id))
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

    buffer_clear(&node, sizeof (struct ext2_node));
    simplewrite(&node, id);
    freenode(id, directory);

    return 1;

}

static void onremoverequest(struct message *message)
{

    struct message_removerequest *request = message_validate(MESSAGE_REMOVEREQUEST, message->data, message->length);
    struct message_removeresponse response;

    response.status = (request) ? removenode(request->parent, request->id) : 0;

    channel_send(0, message->source, MESSAGE_REMOVERESPONSE, sizeof (struct message_removeresponse), &response);

}

static void onstatrequest(struct message *message)
{

    unsigned char data[MESSAGE_SIZE];
    struct message_statrequest *request = message_validate(MESSAGE_STATREQUEST, message->data, message->length);
    struct message_statresponse *response = (struct message_statresponse *)data;

    response->count = (request) ? stat(request->id, (struct record *)(response + 1)) : 0;

    channel_send(0, message->source, MESSAGE_STATRESPONSE, sizeof (struct message_statresponse) + response->count, data);

}

static void onwalkrequest(struct message *message)
{

    struct message_walkrequest *request = message_validate(MESSAGE_WALKREQUEST, message->data, message->length);
    struct message_walkresponse response;

    response.id = (request) ? walk((request->parent) ? request->parent : 2, (char *)(request + 1), request->length) : 0;

    channel_send(0, message->source, MESSAGE_WALKRESPONSE, sizeof (struct message_walkresponse), &response);

}

static void onwriterequest(struct message *message)
{

    struct message_writerequest *request = message_validate(MESSAGE_WRITEREQUEST, message->data, message->length);
    struct message_writeresponse response;
    struct ext2_node node;

    response.count = 0;

    if (request)
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

    channel_send(0, message->source, MESSAGE_WRITERESPONSE, sizeof (struct message_writeresponse), &response);

}

static void onmain(struct message *message)
{

    block = channel_lookup(option_getstring("block-service"));
    partoffset = option_getdecimal("partoffset");

    if (block)
    {

        channel_send(1, block, MESSAGE_INFO, 0, 0);

        if (!channel_wait(1, block, MESSAGE_BLOCKINFO, sizeof (struct message_blockinfo), &blockinfo))
            return;

        readsuperblock();

        if (ext2_validate((struct ext2_superblock *)blockinfo.buffer))
        {

            if (call_announce(0, cstring_length(option_getstring("service")), option_getstring("service")) != MESSAGE_OK)
            {

                channel_send_fmt(0, message->source, MESSAGE_ERROR, "Could not announce service: %s\n", option_getstring("service"));

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
    option_add("partoffset", "0");
    channel_bind(MESSAGE_MAIN, onmain);
    channel_bind(MESSAGE_CREATEREQUEST, oncreaterequest);
    channel_bind(MESSAGE_READREQUEST, onreadrequest);
    channel_bind(MESSAGE_REMOVEREQUEST, onremoverequest);
    channel_bind(MESSAGE_STATREQUEST, onstatrequest);
    channel_bind(MESSAGE_WALKREQUEST, onwalkrequest);
    channel_bind(MESSAGE_WRITEREQUEST, onwriterequest);

}

