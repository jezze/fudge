#include <fudge.h>
#include <abi.h>
#include <hash.h>

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
    unsigned int pointer0;
    unsigned int pointer1;
    unsigned int pointer2;
    unsigned int pointer3;
    unsigned int pointer4;
    unsigned int pointer5;
    unsigned int pointer6;
    unsigned int pointer7;
    unsigned int pointer8;
    unsigned int pointer9;
    unsigned int pointer10;
    unsigned int pointer11;
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

static unsigned int isvalid(struct ext2_superblock *superblock)
{

    return superblock->signature == 0xEF53;

}

static void readsuperblock(struct ext2_superblock *sb)
{

    unsigned char data[1024];

    read(data, 1024, 1, 1024);
    buffer_copy(sb, data, sizeof (struct ext2_superblock));

}

static void readblockgroup(struct ext2_blockgroup *bg, struct ext2_superblock *sb, unsigned int blocksize, unsigned int blockindex, unsigned int blockgroup)
{

    unsigned char data[4096];

    read(data, 4096, (blocksize == 1024) ? 2 : 1, blocksize);
    buffer_copy(bg, data, sizeof (struct ext2_blockgroup));

}

static void readnode(struct ext2_node *node, struct ext2_superblock *sb, struct ext2_blockgroup *bg, unsigned int blocksize, unsigned int nodeindex)
{

    unsigned char data[4096];

    read(data, 4096, bg->blockTableAddress, blocksize);
    buffer_copy(node, data + nodeindex * sb->nodeSize, sizeof (struct ext2_node));

}

static void printsuperblock(unsigned int source, struct ext2_superblock *superblock)
{

    channel_send_fmt1(0, source, EVENT_DATA, "Node Count: %u\n", &superblock->nodeCount);
    channel_send_fmt1(0, source, EVENT_DATA, "Block Count: %u\n", &superblock->blockCount);
    channel_send_fmt1(0, source, EVENT_DATA, "Block Count Super: %u\n", &superblock->blockCountSuper);
    channel_send_fmt1(0, source, EVENT_DATA, "Superblock Index: %u\n", &superblock->superblockIndex);
    channel_send_fmt1(0, source, EVENT_DATA, "Block Size: %u\n", &superblock->blockSize);
    channel_send_fmt1(0, source, EVENT_DATA, "Fragment Size: %u\n", &superblock->fragmentSize);
    channel_send_fmt1(0, source, EVENT_DATA, "Block Count Group: %u\n", &superblock->blockCountGroup);
    channel_send_fmt1(0, source, EVENT_DATA, "Fragment Count Group: %u\n", &superblock->fragmentCountGroup);
    channel_send_fmt1(0, source, EVENT_DATA, "Node Count Group: %u\n", &superblock->nodeCountGroup);
    channel_send_fmt1(0, source, EVENT_DATA, "Signature: 0x%H4h\n", &superblock->signature);
    channel_send_fmt1(0, source, EVENT_DATA, "Minor: %h\n", &superblock->minorVersion);
    channel_send_fmt1(0, source, EVENT_DATA, "Major: %u\n", &superblock->majorVersion);
    channel_send_fmt1(0, source, EVENT_DATA, "Node Size: %h\n", &superblock->nodeSize);

}

/*
static void printblockgroup(struct ext2_blockgroup *blockgroup)
{

    channel_send_fmt1(0, source, EVENT_DATA, "Block Usage Address: %u\n", &blockgroup->blockUsageAddress);
    channel_send_fmt1(0, source, EVENT_DATA, "Node Usage Address: %u\n", &blockgroup->nodeUsageAddress);
    channel_send_fmt1(0, source, EVENT_DATA, "Block Table Address: %u\n", &blockgroup->blockTableAddress);
    channel_send_fmt1(0, source, EVENT_DATA, "Directory Count: %h\n", &blockgroup->directoryCount);

}

static void printnode(struct ext2_node *node)
{

    channel_send_fmt1(0, source, EVENT_DATA, "Type: 0x%H4h\n", &node->type);
    channel_send_fmt1(0, source, EVENT_DATA, "Flags: 0x%H8u\n", &node->flags);
    channel_send_fmt1(0, source, EVENT_DATA, "Pointer 0: 0x%H8u\n", &node->pointer0);
    channel_send_fmt1(0, source, EVENT_DATA, "Pointer 1: 0x%H8u\n", &node->pointer1);
    channel_send_fmt1(0, source, EVENT_DATA, "Pointer 2: 0x%H8u\n", &node->pointer2);
    channel_send_fmt1(0, source, EVENT_DATA, "Pointer 3: 0x%H8u\n", &node->pointer4);

}

static void printdir(struct ext2_entry *entry, char *name)
{

    unsigned int length = entry->length;

    channel_send_fmt1(0, source, EVENT_DATA, "Inode: %u\n", &entry->node);
    channel_send_fmt1(0, source, EVENT_DATA, "Size: %h\n", &entry->size);
    channel_send_fmt1(0, source, EVENT_DATA, "Length: 0x%H2c\n", &entry->length);
    channel_send_fmt2(0, source, EVENT_DATA, "Name: %w\n", name, &length);

}

static void showinode(unsigned int source, struct event_readrequest *readrequest, struct ext2_superblock *sb)
{

    unsigned int blocksize = (1024 << sb->blockSize);
    unsigned int blockgroup = (readrequest->id - 1) / sb->nodeCountGroup;
    unsigned int nodeindex = (readrequest->id - 1) % sb->nodeCountGroup;
    unsigned int blockindex = (readrequest->id * sb->nodeSize) / blocksize;
    struct ext2_blockgroup bg;
    struct ext2_node node;

    readblockgroup(&bg, sb, blocksize, blockindex, blockgroup);
    readnode(&node, sb, &bg, blocksize, nodeindex);

    if ((node.type & 0xF000) == 0x4000)
    {

        struct ext2_entry *entry;
        unsigned char block[4096];
        unsigned int offset = 0;

        read(block, 4096, node.pointer0, blocksize);

        while (offset < 4096)
        {

            unsigned int length;
            char *name;

            entry = (struct ext2_entry *)(block + offset);
            name = (char *)entry + 8;
            length = entry->length;

            channel_send_fmt3(0, source, EVENT_DATA, "(%u) %w\n", &entry->node, name, &length);

            offset += entry->size;

        }

    }

    if ((node.type & 0xF000) == 0x8000)
    {

        unsigned char block[4096];

        read(block, 4096, node.pointer0, blocksize);
        fs_readresponse(source, readrequest->session, (node.sizeLow < 4096) ? node.sizeLow : 4096, block);

    }

}
*/

static struct ext2_superblock sb;
static unsigned int xxx;

static void simpleread(struct ext2_node *node, unsigned int id)
{

    unsigned int blocksize = (1024 << sb.blockSize);
    unsigned int blockgroup = (id - 1) / sb.nodeCountGroup;
    unsigned int nodeindex = (id - 1) % sb.nodeCountGroup;
    unsigned int blockindex = (id * sb.nodeSize) / blocksize;
    struct ext2_blockgroup bg;

    readblockgroup(&bg, &sb, blocksize, blockindex, blockgroup);
    readnode(node, &sb, &bg, blocksize, nodeindex);

}

static void onlistrequest(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_readrequest *listrequest = mdata;
    struct ext2_node node;

    channel_send_fmt0(0, xxx, EVENT_DATA, "On list request\n");
    simpleread(&node, listrequest->id);

    if (listrequest->offset > 0)
    {

        struct event_listresponse response;

        response.nrecords = 0;

        channel_send(0, source, EVENT_LISTRESPONSE, sizeof (struct event_listresponse), &response);

    }

    if ((node.type & 0xF000) == 0x4000)
    {

        struct {struct event_listresponse header; struct record records[8];} response;
        unsigned char data[4096];
        unsigned int offset = 0;
        struct record records[8];
        unsigned int nrecords = 3;
        unsigned int i;

        read(data, 4096, node.pointer0, (1024 << sb.blockSize));

        for (i = 0; i < nrecords; i++)
        {

            struct ext2_entry *entry = (struct ext2_entry *)(data + offset);
            struct record *record = &records[i];

            record->id = entry->node;
            record->size = 0; /* can not be determined */
            record->type = (entry->type == 2) ? RECORD_TYPE_DIRECTORY : RECORD_TYPE_NORMAL;
            record->length = buffer_write(record->name, RECORD_NAMESIZE, (char *)entry + 8, entry->length, 0);

            offset += entry->size;

        }

        response.header.nrecords = nrecords;

        buffer_write(response.records, sizeof (struct record) * 8, records, sizeof (struct record) * nrecords, 0);
        channel_send(0, source, EVENT_LISTRESPONSE, sizeof (struct event_listresponse) + sizeof (struct record) * response.header.nrecords, &response);

    }

    else
    {

        channel_send_fmt1(0, source, EVENT_ERROR, "Not a directory: %u\n", &listrequest->id);

    }

}

static void onreadrequest(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_readrequest *readrequest = mdata;
    struct ext2_node node;

    channel_send_fmt0(0, xxx, EVENT_DATA, "On read request\n");
    simpleread(&node, readrequest->id);

    if ((node.type & 0xF000) == 0x8000)
    {

        /*
        struct {struct event_readresponse header; char data[64];} response;
        unsigned char data[4096];

        read(data, 4096, node.pointer0, (1024 << sb.blockSize));
        response.header.count = buffer_write(response.data, 64, data, (node.sizeLow < 64) ? node.sizeLow : 64, 0);

        channel_send(0, source, EVENT_READRESPONSE, sizeof (struct event_readresponse) + response.header.count, &response);
        */

    }

    else
    {

        channel_send_fmt1(0, source, EVENT_ERROR, "Not a regular file: %u\n", &readrequest->id);

    }

}

static void onwalkrequest(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_walkrequest *walkrequest = mdata;
    unsigned int id = (walkrequest->parent) ? walkrequest->parent : 2;
    char *path = (char *)(walkrequest + 1);
    struct ext2_node node;

    channel_send_fmt0(0, xxx, EVENT_DATA, "On walk request\n");

    if (!walkrequest->length)
    {

        struct event_walkresponse response;

        response.id = id;

        channel_send(0, source, EVENT_WALKRESPONSE, sizeof (struct event_walkresponse), &response);

        return;

    }

    simpleread(&node, id);

    if ((node.type & 0xF000) == 0x4000)
    {

        unsigned char data[4096];
        unsigned int offset = 0;

        channel_send_fmt0(0, xxx, EVENT_DATA, "Read data\n");

        read(data, 4096, node.pointer0, (1024 << sb.blockSize));

        channel_send_fmt0(0, xxx, EVENT_DATA, "Read complete\n");

        while (offset < 4096)
        {

            struct ext2_entry *entry = (struct ext2_entry *)(data + offset);

            channel_send_fmt0(0, xxx, EVENT_DATA, "Loop entries\n");

            if (entry->length == walkrequest->length && buffer_match((char *)entry + 8, path, entry->length))
            {

                struct event_walkresponse response;

                response.id = entry->node;

                channel_send(0, source, EVENT_WALKRESPONSE, sizeof (struct event_walkresponse), &response);

                break;

            }

            offset += entry->size;

        }

    }

    else
    {

        channel_send_fmt1(0, source, EVENT_ERROR, "Not a directory: %u\n", &id);

    }

}

static void onwriterequest(unsigned int source, void *mdata, unsigned int msize)
{

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int target = channel_lookup(option_getstring("block-service"));

    channel_send(0, target, EVENT_LINK, 0, 0);
    readsuperblock(&sb);

    xxx = source;

    if (isvalid(&sb))
    {

        printsuperblock(source, &sb);
        call_announce(0, djb_hash(4, "ext2"));

        while (channel_process(0));

        printsuperblock(source, &sb);

    }

    channel_send(0, target, EVENT_UNLINK, 0, 0);

}

void init(void)
{

    option_add("block-service", "block");
    option_add("partoffset", "2048");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_LISTREQUEST, onlistrequest);
    channel_bind(EVENT_READREQUEST, onreadrequest);
    channel_bind(EVENT_WALKREQUEST, onwalkrequest);
    channel_bind(EVENT_WRITEREQUEST, onwriterequest);

    while (channel_process(0));

}

