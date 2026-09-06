#include <fudge.h>
#include <abi.h>
#include <hash.h>

#define EXT2_MAXBLOCKSIZE               4096

struct ext2_superblock
{

    unsigned int nodeCount;
    unsigned int blockCount;
    unsigned int blockCountSuper;
    unsigned int blockCountUnalloc;
    unsigned int nodeCountUnalloc;
    unsigned int superblockIndex;
    unsigned int blockSize;
    unsigned int fragmentSize;
    unsigned int blockCountGroup;
    unsigned int fragmentCountGroup;
    unsigned int nodeCountGroup;
    unsigned int lastMountTime;
    unsigned int lastWriteTime;
    unsigned short currentCheckCount;
    unsigned short maxCheckCount;
    unsigned short signature;
    unsigned short state;
    unsigned short error;
    unsigned short minorVersion;
    unsigned int lastCheckTime;
    unsigned int forcedIntervalTime;
    unsigned int osId;
    unsigned int majorVersion;
    unsigned short userId;
    unsigned short groupId;
    unsigned int firstUnreservedNode;
    unsigned short nodeSize;
    unsigned short superblockGroup;
    unsigned int optionalAttributes;
    unsigned int requiredFeatures;
    unsigned int unsupportedFeatures;
    unsigned char system[16];
    unsigned char volume[16];
    unsigned char lastmount[64];
    unsigned int algorithm;
    unsigned char filePreallocBlocks;
    unsigned char directoryPreallocBlocks;
    unsigned short unused;
    unsigned char journal[16];
    unsigned int journalNode;
    unsigned int journalDevice;
    unsigned int firstOrphan;

} __attribute__((packed));

struct ext2_blockgroup
{

    unsigned int blockUsageAddress;
    unsigned int nodeUsageAddress;
    unsigned int blockTableAddress;
    unsigned short blockCountUnalloc;
    unsigned short nodeCountUnalloc;
    unsigned short directoryCount;
    unsigned char unused[14];

} __attribute__((packed));

struct ext2_node
{

    unsigned short type;
    unsigned short userId;
    unsigned int sizeLow;
    unsigned int lastAccessTime;
    unsigned int creationTime;
    unsigned int lastModificationTime;
    unsigned int deletionTime;
    unsigned short groupId;
    unsigned short hardCount;
    unsigned int sectorCount;
    unsigned int flags;
    unsigned int osSpecific;
    unsigned int pointer[12];
    unsigned int singlyIndirectPointer;
    unsigned int doublyIndirectPointer;
    unsigned int tripplyIndirectPointer;
    unsigned int generation;
    unsigned int extendedAttributes;
    unsigned int sizeHigh;
    unsigned int fragmentBlockAddress;

} __attribute__((packed));

struct ext2_entry
{

    unsigned int node;
    unsigned short size;
    unsigned char length;
    unsigned char type;

} __attribute__((packed));

static unsigned int ext2_validate(struct ext2_superblock *superblock)
{

    return superblock->signature == 0xEF53;

}

static unsigned int read(void *buffer, unsigned int count, unsigned int sector, unsigned int blocksize)
{

    unsigned int target = channel_lookup(option_getstring("block-service"));

    if (target)
    {

        struct event_blockrequest blockrequest;
        struct message message;

        blockrequest.sector = option_getdecimal("partoffset") + sector * (blocksize / 512);
        blockrequest.count = count;

        channel_send(0, target, EVENT_BLOCKREQUEST, sizeof (struct event_blockrequest), &blockrequest);
        channel_poll(0, target, EVENT_BLOCKRESPONSE, &message);

        return buffer_read(buffer, count, message_data(&message, 0), message.length, 0);

    }

    return 0;

}

static void readsuperblock(struct ext2_superblock *sb)
{

    unsigned char data[1024];

    read(data, 1024, 1, 1024);
    buffer_copy(sb, data, sizeof (struct ext2_superblock));

}

static void readblockgroup(struct ext2_blockgroup *bg, unsigned int blocksize, unsigned int blockgroup)
{

    unsigned int perblock = blocksize / sizeof (struct ext2_blockgroup);
    unsigned int sector = 1 + blockgroup / perblock;
    unsigned int offset = (blockgroup % perblock) * sizeof (struct ext2_blockgroup);
    unsigned char data[EXT2_MAXBLOCKSIZE];

    read(data, EXT2_MAXBLOCKSIZE, sector, blocksize);
    buffer_copy(bg, data + offset, sizeof (struct ext2_blockgroup));

}

static void readnode(struct ext2_node *node, unsigned int blocktable, unsigned int blocksize, unsigned int nodeindex, unsigned int nodesize)
{

    unsigned int perblock = blocksize / nodesize;
    unsigned int sector = blocktable + nodeindex / perblock;
    unsigned int offset = (nodeindex % perblock) * nodesize;
    unsigned char data[EXT2_MAXBLOCKSIZE];

    read(data, EXT2_MAXBLOCKSIZE, sector, blocksize);
    buffer_copy(node, data + offset, sizeof (struct ext2_node));

}

static struct ext2_superblock sb;

static void simpleread(struct ext2_node *node, unsigned int id)
{

    unsigned int blocksize = (1024 << sb.blockSize);
    unsigned int blockgroup = (id - 1) / sb.nodeCountGroup;
    unsigned int nodeindex = (id - 1) % sb.nodeCountGroup;
    struct ext2_blockgroup bg;

    readblockgroup(&bg, blocksize, blockgroup);
    readnode(node, bg.blockTableAddress, blocksize, nodeindex, sb.nodeSize);

}

static unsigned int getindirect(unsigned int sector, unsigned int index, unsigned int blocksize)
{

    if (sector)
    {

        unsigned int table[1024];

        read(table, blocksize, sector, blocksize);

        return table[index];

    }

    return 0;

}

static unsigned int getsector(struct ext2_node *node, unsigned int index, unsigned int blocksize)
{

    unsigned int perblock = blocksize / sizeof (unsigned int);
    unsigned int singlestart = 12;
    unsigned int doublestart = singlestart + perblock;
    unsigned int triplestart = doublestart + perblock * perblock;

    if (index < singlestart)
        return node->pointer[index];

    if (index < doublestart)
        return getindirect(node->singlyIndirectPointer, index - singlestart, blocksize);

    if (index < triplestart)
    {

        unsigned int relative = index - doublestart;
        unsigned int outer = relative / perblock;
        unsigned int inner = relative % perblock;

        return getindirect(getindirect(node->doublyIndirectPointer, outer, blocksize), inner, blocksize);

    }

    {

        unsigned int relative = index - triplestart;
        unsigned int outer = relative / (perblock * perblock);
        unsigned int mid = (relative / perblock) % perblock;
        unsigned int inner = relative % perblock;

        return getindirect(getindirect(getindirect(node->tripplyIndirectPointer, outer, blocksize), mid, blocksize), inner, blocksize);

    }

}

static void onreadrequest(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned char data[MESSAGE_SIZE];
    struct event_readrequest *request = mdata;
    struct event_readresponse *response = (struct event_readresponse *)data;
    unsigned int capacity = MESSAGE_SIZE - sizeof (struct event_readresponse);
    unsigned int blocksize = (1024 << sb.blockSize);
    unsigned char block[EXT2_MAXBLOCKSIZE];
    struct ext2_node node;

    simpleread(&node, request->id);

    response->count = 0;

    switch (node.type & 0xF000)
    {

    case 0x4000:
        {

            unsigned int offset = request->offset;
            struct record *records = (struct record *)(response + 1);
            unsigned int i = 0;

            while (offset < node.sizeLow && (i + 1) * sizeof (struct record) <= capacity)
            {

                unsigned int blockindex = offset / blocksize;
                unsigned int blockoffset = offset % blocksize;
                unsigned int sector = getsector(&node, blockindex, blocksize);
                struct ext2_entry *entry = (struct ext2_entry *)(block + blockoffset);

                if (!sector)
                    break;

                read(block, EXT2_MAXBLOCKSIZE, sector, blocksize);

                if (!entry->size)
                    break;

                if (entry->node)
                {

                    struct record *record = &records[i];

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

                    i++;
                    response->count += sizeof (struct record);

                }

                offset += entry->size;

            }

        }

        channel_send(0, source, EVENT_READRESPONSE, sizeof (struct event_readresponse) + response->count, data);

        break;

    case 0x8000:
        {

            if (request->offset < node.sizeLow)
            {

                unsigned int remaining = node.sizeLow - request->offset;
                unsigned int blockindex = request->offset / blocksize;
                unsigned int blockoffset = request->offset % blocksize;
                unsigned int sector = getsector(&node, blockindex, blocksize);
                unsigned int count = capacity;

                if (count > request->count)
                    count = request->count;

                if (count > remaining)
                    count = remaining;

                if (count > blocksize - blockoffset)
                    count = blocksize - blockoffset;

                if (sector)
                {

                    read(block, EXT2_MAXBLOCKSIZE, sector, blocksize);

                    response->count = buffer_write(data + sizeof (struct event_readresponse), capacity, block + blockoffset, count, 0);

                }

            }

        }

        channel_send(0, source, EVENT_READRESPONSE, sizeof (struct event_readresponse) + response->count, data);
 
        break;

    }

}

static unsigned int matchentry(struct ext2_node *node, unsigned int blocksize, char *name, unsigned int namelength)
{

    unsigned char block[EXT2_MAXBLOCKSIZE];
    unsigned int offset = 0;

    while (offset < node->sizeLow)
    {

        unsigned int blockindex = offset / blocksize;
        unsigned int blockoffset = offset % blocksize;
        unsigned int sector = getsector(node, blockindex, blocksize);
        struct ext2_entry *entry;

        if (!sector)
            break;

        read(block, EXT2_MAXBLOCKSIZE, sector, blocksize);

        entry = (struct ext2_entry *)(block + blockoffset);

        if (!entry->size)
            break;

        if (entry->node && namelength == entry->length && buffer_match(entry + 1, name, entry->length))
            return entry->node;

        offset += entry->size;

    }

    return 0;

}

static void onwalkrequest(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_walkrequest *walkrequest = mdata;
    unsigned int id = (walkrequest->parent) ? walkrequest->parent : 2;
    char *path = (char *)(walkrequest + 1);
    unsigned int length = walkrequest->length;
    unsigned int offset = 0;
    struct event_walkresponse response;


    while (offset < length && id)
    {

        unsigned int blocksize = (1024 << sb.blockSize);
        unsigned int seglength = 0;
        struct ext2_node node;

        while (offset + seglength < length && path[offset + seglength] != '/')
            seglength++;

        if (seglength)
        {

            simpleread(&node, id);

            if ((node.type & 0xF000) != 0x4000)
            {

                id = 0;

                break;

            }

            id = matchentry(&node, blocksize, path + offset, seglength);

        }

        offset += seglength + 1;

    }

    response.id = id;

    channel_send(0, source, EVENT_WALKRESPONSE, sizeof (struct event_walkresponse), &response);

}

static void onwriterequest(unsigned int source, void *mdata, unsigned int msize)
{

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int target = channel_lookup(option_getstring("block-service"));

    channel_send(0, target, EVENT_LINK, 0, 0);
    readsuperblock(&sb);

    if (ext2_validate(&sb))
    {

        call_announce(0, djb_hash(4, "ext2"));

        while (channel_process(0));

    }

    channel_send(0, target, EVENT_UNLINK, 0, 0);

}

void init(void)
{

    option_add("block-service", "block");
    option_add("partoffset", "2048");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_READREQUEST, onreadrequest);
    channel_bind(EVENT_WALKREQUEST, onwalkrequest);
    channel_bind(EVENT_WRITEREQUEST, onwriterequest);

    while (channel_process(0));

}

