#include <fudge.h>
#include <abi.h>
#include <disk.h>
#include <hash.h>

static struct event_blockinfo blockinfo;

static unsigned int sendblockreadrequest(unsigned int count, unsigned int sector, unsigned int blocksize)
{

    unsigned int target = channel_lookup(option_getstring("block-service"));

    if (target)
    {

        struct event_blockrequest request;
        struct event_blockresponse response;

        request.offset = option_getdecimal("partoffset") + sector * blocksize;
        request.count = count;

        channel_send(0, target, EVENT_BLOCKREADREQUEST, sizeof (struct event_blockrequest), &request);
        channel_wait(0, target, EVENT_BLOCKREADRESPONSE, sizeof (struct event_blockresponse), &response);

        return response.count;

    }

    return 0;

}

static unsigned int sendblockwriterequest(unsigned int count, unsigned int sector, unsigned int blocksize)
{

    unsigned int target = channel_lookup(option_getstring("block-service"));

    if (target)
    {

        struct event_blockrequest request;
        struct event_blockresponse response;

        request.offset = option_getdecimal("partoffset") + sector * blocksize;
        request.count = count;

        channel_send(0, target, EVENT_BLOCKWRITEREQUEST, sizeof (struct event_blockrequest), &request);
        channel_wait(0, target, EVENT_BLOCKWRITERESPONSE, sizeof (struct event_blockresponse), &response);

        return response.count;

    }

    return 0;

}

static struct ext2_superblock sb;
static unsigned int blocksize;

static void readsuperblock(void)
{

    sendblockreadrequest(1024, 1, 1024);
    buffer_copy(&sb, (void *)blockinfo.buffer, sizeof (struct ext2_superblock));

    blocksize = (1024 << sb.blockSize);

}

static void writesuperblock(void)
{

    sendblockreadrequest(1024, 1, 1024);
    buffer_copy((void *)blockinfo.buffer, &sb, sizeof (struct ext2_superblock));
    sendblockwriterequest(1024, 1, 1024);

}

static void readblockgroup(struct ext2_blockgroup *bg, unsigned int start, unsigned int index)
{

    unsigned int slots = blocksize / sizeof (struct ext2_blockgroup);
    unsigned int sector = start + index / slots;
    unsigned int offset = (index % slots) * sizeof (struct ext2_blockgroup);

    sendblockreadrequest(EXT2_MAXBLOCKSIZE, sector, blocksize);
    buffer_copy(bg, (char *)blockinfo.buffer + offset, sizeof (struct ext2_blockgroup));

}

static void writeblockgroup(struct ext2_blockgroup *bg, unsigned int start, unsigned int index)
{

    unsigned int slots = blocksize / sizeof (struct ext2_blockgroup);
    unsigned int sector = start + index / slots;
    unsigned int offset = (index % slots) * sizeof (struct ext2_blockgroup);

    sendblockreadrequest(EXT2_MAXBLOCKSIZE, sector, blocksize);
    buffer_copy((char *)blockinfo.buffer + offset, bg, sizeof (struct ext2_blockgroup));
    sendblockwriterequest(EXT2_MAXBLOCKSIZE, sector, blocksize);

}

static void readnode(struct ext2_node *node, unsigned int start, unsigned int index)
{

    unsigned int slots = blocksize / sb.nodeSize;
    unsigned int sector = start + index / slots;
    unsigned int offset = (index % slots) * sb.nodeSize;

    sendblockreadrequest(EXT2_MAXBLOCKSIZE, sector, blocksize);
    buffer_copy(node, (char *)blockinfo.buffer + offset, sizeof (struct ext2_node));

}

static void writenode(struct ext2_node *node, unsigned int start, unsigned int index)
{

    unsigned int slots = blocksize / sb.nodeSize;
    unsigned int sector = start + index / slots;
    unsigned int offset = (index % slots) * sb.nodeSize;

    sendblockreadrequest(EXT2_MAXBLOCKSIZE, sector, blocksize);
    buffer_copy((char *)blockinfo.buffer + offset, node, sizeof (struct ext2_node));
    sendblockwriterequest(EXT2_MAXBLOCKSIZE, sector, blocksize);

}

static void simpleread(struct ext2_node *node, unsigned int id)
{

    unsigned int igroup = (id - 1) / sb.nodeCountGroup;
    unsigned int inode = (id - 1) % sb.nodeCountGroup;
    struct ext2_blockgroup bg;

    readblockgroup(&bg, 1, igroup);
    readnode(node, bg.blockTableAddress, inode);

}

static void simplewrite(struct ext2_node *node, unsigned int id)
{

    unsigned int igroup = (id - 1) / sb.nodeCountGroup;
    unsigned int inode = (id - 1) % sb.nodeCountGroup;
    struct ext2_blockgroup bg;

    readblockgroup(&bg, 1, igroup);
    writenode(node, bg.blockTableAddress, inode);

}

static unsigned int allocblock(unsigned int blockgroup)
{

    struct ext2_blockgroup bg;
    unsigned char *bitmap = (unsigned char *)blockinfo.buffer;
    unsigned int i;

    readblockgroup(&bg, 1, blockgroup);
    sendblockreadrequest(EXT2_MAXBLOCKSIZE, bg.blockUsageAddress, blocksize);

    for (i = 0; i < sb.blockCountGroup; i++)
    {

        if (!(bitmap[i / 8] & (1 << (i % 8))))
        {

            bitmap[i / 8] |= 1 << (i % 8);

            sendblockwriterequest(EXT2_MAXBLOCKSIZE, bg.blockUsageAddress, blocksize);

            bg.blockCountUnalloc--;

            writeblockgroup(&bg, 1, blockgroup);

            sb.blockCountUnalloc--;

            writesuperblock();

            return sb.superblockIndex + blockgroup * sb.blockCountGroup + i;

        }

    }

    return 0;

}

static unsigned int allocnode(unsigned int igroup)
{

    struct ext2_blockgroup bg;
    unsigned char *bitmap = (unsigned char *)blockinfo.buffer;
    unsigned int i;

    readblockgroup(&bg, 1, igroup);
    sendblockreadrequest(EXT2_MAXBLOCKSIZE, bg.nodeUsageAddress, blocksize);

    for (i = 0; i < sb.nodeCountGroup; i++)
    {

        if (!(bitmap[i / 8] & (1 << (i % 8))))
        {

            bitmap[i / 8] |= 1 << (i % 8);

            sendblockwriterequest(EXT2_MAXBLOCKSIZE, bg.nodeUsageAddress, blocksize);

            bg.nodeCountUnalloc--;

            writeblockgroup(&bg, 1, igroup);

            sb.nodeCountUnalloc--;

            writesuperblock();

            return igroup * sb.nodeCountGroup + i + 1;

        }

    }

    return 0;

}

static unsigned int getindirect(unsigned int sector, unsigned int index)
{

    if (sector)
    {

        unsigned int *table = (unsigned int *)blockinfo.buffer;

        sendblockreadrequest(blocksize, sector, blocksize);

        return table[index];

    }

    return 0;

}

static unsigned int getsector(struct ext2_node *node, unsigned int index)
{

    unsigned int slots = blocksize / 4;
    unsigned int singlestart = 12;
    unsigned int doublestart = singlestart + slots;
    unsigned int triplestart = doublestart + slots * slots;

    if (index < singlestart)
        return node->pointer[index];

    if (index < doublestart)
        return getindirect(node->singlyIndirectPointer, index - singlestart);

    if (index < triplestart)
    {

        unsigned int relative = index - doublestart;
        unsigned int outer = relative / slots;
        unsigned int inner = relative % slots;

        return getindirect(getindirect(node->doublyIndirectPointer, outer), inner);

    }

    {

        unsigned int relative = index - triplestart;
        unsigned int outer = relative / (slots * slots);
        unsigned int mid = (relative / slots) % slots;
        unsigned int inner = relative % slots;

        return getindirect(getindirect(getindirect(node->tripplyIndirectPointer, outer), mid), inner);

    }

}

static unsigned int allocsector(struct ext2_node *node, unsigned int index, unsigned int igroup)
{

    unsigned int slots = blocksize / 4;

    if (index < 12)
    {

        unsigned int sector = allocblock(igroup);

        if (sector)
            node->pointer[index] = sector;

        return sector;

    }

    index -= 12;

    if (index < slots)
    {

        unsigned int *table;
        unsigned int sector;

        if (!node->singlyIndirectPointer)
        {

            unsigned int tableblock = allocblock(igroup);

            if (!tableblock)
                return 0;

            buffer_clear((void *)blockinfo.buffer, EXT2_MAXBLOCKSIZE);
            sendblockwriterequest(EXT2_MAXBLOCKSIZE, tableblock, blocksize);

            node->singlyIndirectPointer = tableblock;

        }

        sector = allocblock(igroup);

        if (!sector)
            return 0;

        sendblockreadrequest(EXT2_MAXBLOCKSIZE, node->singlyIndirectPointer, blocksize);

        table = (unsigned int *)blockinfo.buffer;
        table[index] = sector;

        sendblockwriterequest(EXT2_MAXBLOCKSIZE, node->singlyIndirectPointer, blocksize);

        return sector;

    }

    return 0;

}

static void getrecord(struct ext2_entry *entry, struct record *record, unsigned int offset)
{

    record->id = entry->node;
    record->size = 0;
    record->offset = offset + entry->size;
    record->length = buffer_write(record->name, RECORD_NAMESIZE, entry + 1, entry->length, 0);

    switch (entry->type)
    {

    case 1:
        record->type = RECORD_TYPE_NORMAL;

        break;

    case 2:
        record->type = RECORD_TYPE_DIRECTORY;

        break;

    }

}

static unsigned int matchentry(struct ext2_node *node, char *name, unsigned int length)
{

    unsigned int offset = 0;

    while (offset < node->sizeLow)
    {

        unsigned int blockindex = offset / blocksize;
        unsigned int blockoffset = offset % blocksize;
        unsigned int sector = getsector(node, blockindex);
        struct ext2_entry *entry = (struct ext2_entry *)((char *)blockinfo.buffer + blockoffset);

        if (!sector)
            break;

        sendblockreadrequest(EXT2_MAXBLOCKSIZE, sector, blocksize);

        if (!entry->size)
            break;

        if (entry->node && entry->length == length && buffer_match(entry + 1, name, length))
            return entry->node;

        offset += entry->size;

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

static unsigned int addentry(struct ext2_node *dir, unsigned int igroup, unsigned int newid, char *name, unsigned int namelength, unsigned int type)
{

    unsigned int realsize = (8 + namelength + 3) & ~3;
    unsigned int offset = 0;

    while (offset < dir->sizeLow)
    {

        unsigned int blockindex = offset / blocksize;
        unsigned int blockoffset = offset % blocksize;
        unsigned int sector = getsector(dir, blockindex);
        struct ext2_entry *entry;
        unsigned int used;

        if (!sector)
            break;

        sendblockreadrequest(EXT2_MAXBLOCKSIZE, sector, blocksize);

        entry = (struct ext2_entry *)((char *)blockinfo.buffer + blockoffset);

        if (!entry->size)
            break;

        used = entry->node ? (8 + entry->length + 3) & ~3 : 0;

        if (entry->size - used >= realsize)
        {

            struct ext2_entry *fresh = (struct ext2_entry *)((char *)entry + used);
            unsigned int freshsize = entry->size - used;

            if (used)
                entry->size = used;

            fresh->node = newid;
            fresh->size = freshsize;
            fresh->length = namelength;
            fresh->type = type;

            buffer_copy(fresh + 1, name, namelength);

            sendblockwriterequest(EXT2_MAXBLOCKSIZE, sector, blocksize);

            return 1;

        }

        offset += entry->size;

    }

    {

        unsigned int blockindex = dir->sizeLow / blocksize;
        unsigned int sector = allocsector(dir, blockindex, igroup);
        struct ext2_entry *fresh;

        if (!sector)
            return 0;

        buffer_clear((void *)blockinfo.buffer, EXT2_MAXBLOCKSIZE);

        fresh = (struct ext2_entry *)blockinfo.buffer;
        fresh->node = newid;
        fresh->size = blocksize;
        fresh->length = namelength;
        fresh->type = type;

        buffer_copy(fresh + 1, name, namelength);

        sendblockwriterequest(EXT2_MAXBLOCKSIZE, sector, blocksize);

        dir->sizeLow += blocksize;

        return 1;

    }

}

static void oncreaterequest(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_createrequest *request = mdata;
    struct event_createresponse response;
    struct ext2_node parent;

    response.id = 0;

    simpleread(&parent, request->parent);

    switch (parent.type & 0xF000)
    {

    case 0x4000:
        {

            unsigned int igroup = (request->parent - 1) / sb.nodeCountGroup;
            unsigned int id = allocnode(igroup);

            if (id)
            {

                struct ext2_node node;

                buffer_clear(&node, sizeof (struct ext2_node));

                node.type = 0x81A4;
                node.hardCount = 1;

                simplewrite(&node, id);

                if (addentry(&parent, igroup, id, (char *)(request + 1), request->count, 1))
                {

                    simplewrite(&parent, request->parent);

                    response.id = id;

                }

            }

        }

        break;

    }

    channel_send(0, source, EVENT_CREATERESPONSE, sizeof (struct event_createresponse), &response);

}

static unsigned int readdirectory(struct ext2_node *node, unsigned int roffset, unsigned int rcount, unsigned int capacity, void *data)
{

    struct record *records = data;
    unsigned int count = 0;
    unsigned int i = 0;

    while (roffset < node->sizeLow && (i + 1) * sizeof (struct record) <= capacity)
    {

        unsigned int blockindex = roffset / blocksize;
        unsigned int blockoffset = roffset % blocksize;
        unsigned int sector = getsector(node, blockindex);
        struct ext2_entry *entry = (struct ext2_entry *)((char *)blockinfo.buffer + blockoffset);

        if (!sector)
            break;

        sendblockreadrequest(EXT2_MAXBLOCKSIZE, sector, blocksize);

        if (!entry->size)
            break;

        if (entry->node)
        {

            getrecord(entry, &records[i], roffset);

            i++;
            count += sizeof (struct record);

        }

        roffset += entry->size;

    }

    return count;

}

static unsigned int readfile(struct ext2_node *node, unsigned int roffset, unsigned int rcount, unsigned int capacity, void *data)
{

    if (roffset < node->sizeLow)
    {

        unsigned int remaining = node->sizeLow - roffset;
        unsigned int blockindex = roffset / blocksize;
        unsigned int blockoffset = roffset % blocksize;
        unsigned int sector = getsector(node, blockindex);
        unsigned int count = capacity;

        if (count > rcount)
            count = rcount;

        if (count > remaining)
            count = remaining;

        if (count > blocksize - blockoffset)
            count = blocksize - blockoffset;

        if (sector)
        {

            sendblockreadrequest(EXT2_MAXBLOCKSIZE, sector, blocksize);

            return buffer_write(data, capacity, (char *)blockinfo.buffer + blockoffset, count, 0);

        }

    }

    return 0;

}

static void onreadrequest(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned char data[MESSAGE_SIZE];
    struct event_readrequest *request = mdata;
    struct event_readresponse *response = (struct event_readresponse *)data;
    struct ext2_node node;

    simpleread(&node, request->id);

    response->count = 0;

    switch (node.type & 0xF000)
    {

    case 0x4000:
        response->count = readdirectory(&node, request->offset, request->count, MESSAGE_SIZE - sizeof (struct event_readresponse), response + 1);

        break;

    case 0x8000:
        response->count = readfile(&node, request->offset, request->count, MESSAGE_SIZE - sizeof (struct event_readresponse), response + 1);

        break;

    }

    channel_send(0, source, EVENT_READRESPONSE, sizeof (struct event_readresponse) + response->count, data);

}

static void onwalkrequest(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_walkrequest *request = mdata;
    struct event_walkresponse response;

    response.id = walk((request->parent) ? request->parent : 2, (char *)(request + 1), request->length);

    channel_send(0, source, EVENT_WALKRESPONSE, sizeof (struct event_walkresponse), &response);

}

static void onwriterequest(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_writerequest *request = mdata;
    struct event_writeresponse response;
    struct ext2_node node;

    simpleread(&node, request->id);

    response.count = 0;

    switch (node.type & 0xF000)
    {

    case 0x8000:
        if (request->offset <= node.sizeLow)
        {

            unsigned int blockindex = request->offset / blocksize;
            unsigned int blockoffset = request->offset % blocksize;
            unsigned int sector = getsector(&node, blockindex);
            unsigned int count = request->count;
            unsigned int allocated = 0;

            if (count > blocksize - blockoffset)
                count = blocksize - blockoffset;

            if (!sector)
            {

                unsigned int igroup = (request->id - 1) / sb.nodeCountGroup;

                sector = allocsector(&node, blockindex, igroup);

                if (sector)
                    allocated = 1;

            }

            if (sector)
            {

                if (allocated)
                    buffer_clear((void *)blockinfo.buffer, EXT2_MAXBLOCKSIZE);
                else
                    sendblockreadrequest(EXT2_MAXBLOCKSIZE, sector, blocksize);

                buffer_copy((char *)blockinfo.buffer + blockoffset, request + 1, count);
                sendblockwriterequest(EXT2_MAXBLOCKSIZE, sector, blocksize);

                response.count = count;

                if (request->offset + count > node.sizeLow)
                {

                    node.sizeLow = request->offset + count;

                    simplewrite(&node, request->id);

                }

            }

        }

        break;

    }

    channel_send(0, source, EVENT_WRITERESPONSE, sizeof (struct event_writeresponse), &response);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int block = channel_lookup(option_getstring("block-service"));

    if (block)
    {

        channel_send(0, block, EVENT_INFO, 0, 0);
        channel_wait(0, block, EVENT_BLOCKINFO, sizeof (struct event_blockinfo), &blockinfo);
        readsuperblock();

        if (ext2_validate(&sb))
        {

            char *service = option_getstring("service");

            call_announce(0, djb_hash(cstring_length(service), service));

            while (channel_process(0));

        }

    }

}

void init(void)
{

    option_add("service", "ext2");
    option_add("block-service", "block");
    option_add("partoffset", "1048576");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_CREATEREQUEST, oncreaterequest);
    channel_bind(EVENT_READREQUEST, onreadrequest);
    channel_bind(EVENT_WALKREQUEST, onwalkrequest);
    channel_bind(EVENT_WRITEREQUEST, onwriterequest);

    while (channel_process(0));

}

