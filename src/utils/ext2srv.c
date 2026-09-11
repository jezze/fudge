#include <fudge.h>
#include <abi.h>
#include <disk.h>
#include <hash.h>

static void *blockbuffer;
static unsigned int blockbuffersize;

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
    buffer_copy(&sb, blockbuffer, sizeof (struct ext2_superblock));

    blocksize = (1024 << sb.blockSize);

}

static void readblockgroup(struct ext2_blockgroup *bg, unsigned int start, unsigned int index)
{

    unsigned int slots = blocksize / sizeof (struct ext2_blockgroup);
    unsigned int sector = start + index / slots;
    unsigned int offset = (index % slots) * sizeof (struct ext2_blockgroup);

    sendblockreadrequest(EXT2_MAXBLOCKSIZE, sector, blocksize);
    buffer_copy(bg, (char *)blockbuffer + offset, sizeof (struct ext2_blockgroup));

}

static void readnode(struct ext2_node *node, unsigned int start, unsigned int index)
{

    unsigned int slots = blocksize / sb.nodeSize;
    unsigned int sector = start + index / slots;
    unsigned int offset = (index % slots) * sb.nodeSize;

    sendblockreadrequest(EXT2_MAXBLOCKSIZE, sector, blocksize);
    buffer_copy(node, (char *)blockbuffer + offset, sizeof (struct ext2_node));

}

static void simpleread(struct ext2_node *node, unsigned int id)
{

    unsigned int igroup = (id - 1) / sb.nodeCountGroup;
    unsigned int inode = (id - 1) % sb.nodeCountGroup;
    struct ext2_blockgroup bg;

    readblockgroup(&bg, 1, igroup);
    readnode(node, bg.blockTableAddress, inode);

}

static unsigned int getindirect(unsigned int sector, unsigned int index)
{

    if (sector)
    {

        unsigned int *table = (unsigned int *)blockbuffer;

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
        struct ext2_entry *entry = (struct ext2_entry *)((char *)blockbuffer + blockoffset);

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

            id = ((node.type & 0xF000) == 0x4000) ? matchentry(&node, path + offset, count) : 0;

            if (!id)
                return 0;

        }

        offset += count + 1;

    }

    return id;

}

static void onreadrequest(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned char data[MESSAGE_SIZE];
    struct event_readrequest *request = mdata;
    struct event_readresponse *response = (struct event_readresponse *)data;
    unsigned int capacity = MESSAGE_SIZE - sizeof (struct event_readresponse);
    struct ext2_node node;

    simpleread(&node, request->id);

    response->count = 0;

    switch (node.type & 0xF000)
    {

    case 0x4000:
        {

            struct record *records = (struct record *)(response + 1);
            unsigned int i = 0;

            while (request->offset < node.sizeLow && (i + 1) * sizeof (struct record) <= capacity)
            {

                unsigned int blockindex = request->offset / blocksize;
                unsigned int blockoffset = request->offset % blocksize;
                unsigned int sector = getsector(&node, blockindex);
                struct ext2_entry *entry = (struct ext2_entry *)((char *)blockbuffer + blockoffset);

                if (!sector)
                    break;

                sendblockreadrequest(EXT2_MAXBLOCKSIZE, sector, blocksize);

                if (!entry->size)
                    break;

                if (entry->node)
                {

                    getrecord(entry, &records[i], request->offset);

                    i++;
                    response->count += sizeof (struct record);

                }

                request->offset += entry->size;

            }

        }


        break;

    case 0x8000:
        if (request->offset < node.sizeLow)
        {

            unsigned int remaining = node.sizeLow - request->offset;
            unsigned int blockindex = request->offset / blocksize;
            unsigned int blockoffset = request->offset % blocksize;
            unsigned int sector = getsector(&node, blockindex);
            unsigned int count = capacity;

            if (count > request->count)
                count = request->count;

            if (count > remaining)
                count = remaining;

            if (count > blocksize - blockoffset)
                count = blocksize - blockoffset;

            if (sector)
            {

                sendblockreadrequest(EXT2_MAXBLOCKSIZE, sector, blocksize);

                response->count = buffer_write(data + sizeof (struct event_readresponse), capacity, (char *)blockbuffer + blockoffset, count, 0);

            }

        }

 
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
        if (request->offset < node.sizeLow)
        {

            unsigned int remaining = node.sizeLow - request->offset;
            unsigned int blockindex = request->offset / blocksize;
            unsigned int blockoffset = request->offset % blocksize;
            unsigned int sector = getsector(&node, blockindex);
            unsigned int count = request->count;

            if (count > remaining)
                count = remaining;

            if (count > blocksize - blockoffset)
                count = blocksize - blockoffset;

            if (sector)
            {

                sendblockreadrequest(EXT2_MAXBLOCKSIZE, sector, blocksize);
                buffer_copy((char *)blockbuffer + blockoffset, request + 1, count);
                sendblockwriterequest(EXT2_MAXBLOCKSIZE, sector, blocksize);

                response.count = count;

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

        struct event_blockinfo info;

        channel_send(0, block, EVENT_INFO, 0, 0);
        channel_wait(0, block, EVENT_BLOCKINFO, sizeof (struct event_blockinfo), &info);

        blockbuffer = (void *)info.buffer;
        blockbuffersize = info.buffersize;

        readsuperblock();

        if (ext2_validate(&sb))
        {

            call_announce(0, djb_hash(4, "ext2"));

            while (channel_process(0));

        }

    }

}

void init(void)
{

    option_add("block-service", "block");
    option_add("partoffset", "1048576");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_READREQUEST, onreadrequest);
    channel_bind(EVENT_WALKREQUEST, onwalkrequest);
    channel_bind(EVENT_WRITEREQUEST, onwriterequest);

    while (channel_process(0));

}

