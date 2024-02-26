#include <fudge.h>
#include <abi.h>

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

static void request_send(unsigned int sector, unsigned int count)
{

    struct event_blockrequest blockrequest;

    blockrequest.sector = sector;
    blockrequest.count = count;

    call_notify(FILE_G5, EVENT_BLOCKREQUEST, sizeof (struct event_blockrequest), &blockrequest);

}

static void request_readblocks(void *buffer, unsigned int count, unsigned int sector, unsigned int nblocks, unsigned int blocksize)
{

    unsigned int diff = blocksize / 512;
    unsigned int total = nblocks * blocksize;
    unsigned int read = 0;
    struct message message;
    char data[MESSAGE_SIZE];

    request_send(option_getdecimal("partoffset") + sector * diff, nblocks * diff);

    while (channel_poll_any(EVENT_DATA, &message, data))
    {

        read += buffer_write(buffer, count, data, message_datasize(&message), read);

        if (read == total)
            break;

    }

}

static unsigned int isvalid(struct ext2_superblock *superblock)
{

    return superblock->signature == 0xEF53;

}

static void readsuperblock(struct ext2_superblock *sb)
{

    unsigned char block[1024];

    request_readblocks(block, 1024, 1, 1, 1024);
    buffer_copy(sb, block, sizeof (struct ext2_superblock));

}

static void readblockgroup(struct ext2_blockgroup *bg, struct ext2_superblock *sb, unsigned int blocksize, unsigned int blockindex, unsigned int blockgroup)
{

    unsigned char block[4096];

    request_readblocks(block, 4096, (blocksize == 1024) ? 2 : 1, 1, blocksize);
    buffer_copy(bg, block, sizeof (struct ext2_blockgroup));

}

static void readnode(struct ext2_node *node, struct ext2_superblock *sb, struct ext2_blockgroup *bg, unsigned int blocksize, unsigned int nodeindex)
{

    unsigned char block[4096];

    request_readblocks(block, 4096, bg->blockTableAddress, 1, blocksize);
    buffer_copy(node, block + nodeindex * sb->nodeSize, sizeof (struct ext2_node));

}

/*
static void printsuperblock(struct ext2_superblock *superblock)
{

    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "Node Count: %u\n", &superblock->nodeCount);
    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "Block Count: %u\n", &superblock->blockCount);
    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "Block Count Super: %u\n", &superblock->blockCountSuper);
    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "Superblock Index: %u\n", &superblock->superblockIndex);
    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "Block Size: %u\n", &superblock->blockSize);
    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "Fragment Size: %u\n", &superblock->fragmentSize);
    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "Block Count Group: %u\n", &superblock->blockCountGroup);
    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "Fragment Count Group: %u\n", &superblock->fragmentCountGroup);
    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "Node Count Group: %u\n", &superblock->nodeCountGroup);
    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "Signature: 0x%H4h\n", &superblock->signature);
    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "Minor: %h\n", &superblock->minorVersion);
    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "Major: %u\n", &superblock->majorVersion);
    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "Node Size: %h\n", &superblock->nodeSize);

}

static void printblockgroup(struct ext2_blockgroup *blockgroup)
{

    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "Block Usage Address: %u\n", &blockgroup->blockUsageAddress);
    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "Node Usage Address: %u\n", &blockgroup->nodeUsageAddress);
    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "Block Table Address: %u\n", &blockgroup->blockTableAddress);
    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "Directory Count: %h\n", &blockgroup->directoryCount);

}

static void printnode(struct ext2_node *node)
{

    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "Type: 0x%H4h\n", &node->type);
    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "Flags: 0x%H8u\n", &node->flags);
    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "Pointer 0: 0x%H8u\n", &node->pointer0);
    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "Pointer 1: 0x%H8u\n", &node->pointer1);
    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "Pointer 2: 0x%H8u\n", &node->pointer2);
    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "Pointer 3: 0x%H8u\n", &node->pointer4);

}

static void printdir(struct ext2_entry *entry, char *name)
{

    unsigned int length = entry->length;

    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "Inode: %u\n", &entry->node);
    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "Size: %h\n", &entry->size);
    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "Length: 0x%H2c\n", &entry->length);
    channel_send_fmt2(CHANNEL_DEFAULT, EVENT_DATA, "Name: %w\n", name, &length);

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

        request_readblocks(block, 4096, node.pointer0, 1, blocksize);

        while (offset < 4096)
        {

            unsigned int length;
            char *name;

            entry = (struct ext2_entry *)(block + offset);
            name = (char *)entry + 8;
            length = entry->length;

            channel_send_fmt3(source, EVENT_DATA, "(%u) %w\n", &entry->node, name, &length);

            offset += entry->size;

        }

    }

    if ((node.type & 0xF000) == 0x8000)
    {

        unsigned char block[4096];

        request_readblocks(block, 4096, node.pointer0, 1, blocksize);
        sendreadresponse(source, readrequest->session, (node.sizeLow < 4096) ? node.sizeLow : 4096, block);

    }

}
*/

static void sendlistresponse(unsigned int source, unsigned int session, unsigned int nrecords, struct record *records)
{

    struct {struct event_listresponse listresponse; struct record records[8];} message;

    message.listresponse.session = session;
    message.listresponse.nrecords = nrecords;

    buffer_write(message.records, sizeof (struct record) * 8, records, sizeof (struct record) * nrecords, 0);
    channel_send_buffer(source, EVENT_LISTRESPONSE, sizeof (struct event_listresponse) + sizeof (struct record) * message.listresponse.nrecords, &message);

}

static void sendreadresponse(unsigned int source, unsigned int session, unsigned int count, void *buffer)
{

    struct {struct event_readresponse readresponse; char data[64];} message;

    message.readresponse.session = session;
    message.readresponse.count = count;

    buffer_write(message.data, 64, buffer, message.readresponse.count, 0);
    channel_send_buffer(source, EVENT_READRESPONSE, sizeof (struct event_readresponse) + message.readresponse.count, &message);

}

static void sendwalkresponse(unsigned int source, unsigned int session, unsigned int id)
{

    struct event_walkresponse walkresponse;

    walkresponse.session = session;
    walkresponse.id = id;

    channel_send_buffer(source, EVENT_WALKRESPONSE, sizeof (struct event_walkresponse), &walkresponse);

}

static struct ext2_superblock sb;

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

    simpleread(&node, listrequest->id);

    if ((node.type & 0xF000) == 0x4000)
    {

        unsigned char block[4096];
        unsigned int offset = 0;
        struct record records[8];
        unsigned int nrecords = 0;

        request_readblocks(block, 4096, node.pointer0, 1, (1024 << sb.blockSize));

        while (offset < 4096)
        {

            struct ext2_entry *entry = (struct ext2_entry *)(block + offset);
            struct record *record = &records[nrecords];

            record->id = entry->node;
            record->size = 0; /* can not be determined */
            record->type = (entry->type == 2) ? RECORD_TYPE_DIRECTORY : RECORD_TYPE_NORMAL;
            record->length = buffer_write(record->name, RECORD_NAMESIZE, (char *)entry + 8, entry->length, 0);

            if (nrecords < 8)
                nrecords++;
            else
                break;

            offset += entry->size;

        }

        sendlistresponse(source, listrequest->session, nrecords, records);

    }

    else
    {

        channel_send_fmt1(CHANNEL_DEFAULT, EVENT_ERROR, "Not a directory: %u\n", &listrequest->id);

    }

}

static void onreadrequest(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_readrequest *readrequest = mdata;
    struct ext2_node node;

    simpleread(&node, readrequest->id);

    if ((node.type & 0xF000) == 0x8000)
    {

        unsigned char block[4096];

        request_readblocks(block, 4096, node.pointer0, 1, (1024 << sb.blockSize));
        sendreadresponse(source, readrequest->session, (node.sizeLow < 4096) ? node.sizeLow : 4096, block);

    }

    else
    {

        channel_send_fmt1(CHANNEL_DEFAULT, EVENT_ERROR, "Not a regular file: %u\n", &readrequest->id);

    }

}

static void onwalkrequest(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_walkrequest *walkrequest = mdata;
    unsigned int id = (walkrequest->parent) ? walkrequest->parent : 2;
    char *path = (char *)(walkrequest + 1);
    struct ext2_node node;

    if (!walkrequest->length)
    {

        sendwalkresponse(source, walkrequest->session, id);

        return;

    }

    simpleread(&node, id);

    if ((node.type & 0xF000) == 0x4000)
    {

        unsigned char block[4096];
        unsigned int offset = 0;

        request_readblocks(block, 4096, node.pointer0, 1, (1024 << sb.blockSize));

        while (offset < 4096)
        {

            struct ext2_entry *entry = (struct ext2_entry *)(block + offset);

            if (entry->length == walkrequest->length && buffer_match((char *)entry + 8, path, entry->length))
            {

                sendwalkresponse(source, walkrequest->session, entry->node);

                break;

            }

            offset += entry->size;

        }

    }

    else
    {

        channel_send_fmt1(CHANNEL_DEFAULT, EVENT_ERROR, "Not a directory: %u\n", &id);

    }

}

static void onwriterequest(unsigned int source, void *mdata, unsigned int msize)
{

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    if (!call_walk_absolute(FILE_G5, option_getstring("volume")))
        PANIC();

    call_link(FILE_G5, 8001);
    readsuperblock(&sb);

    if (isvalid(&sb))
    {

        call_announce(option_getdecimal("fs-service"));

        while (channel_process());

    }

    call_unlink(FILE_G5);

}

void init(void)
{

    option_add("fs-service", "1111");
    option_add("volume", "system:block/if.0/data");
    option_add("partoffset", "2048");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_LISTREQUEST, onlistrequest);
    channel_bind(EVENT_READREQUEST, onreadrequest);
    channel_bind(EVENT_WALKREQUEST, onwalkrequest);
    channel_bind(EVENT_WRITEREQUEST, onwriterequest);

}

