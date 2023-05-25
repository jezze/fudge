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

static void request_readblocks(void *buffer, unsigned int count, unsigned int sector, unsigned int nblocks)
{

    unsigned int total = nblocks * 512;
    unsigned int read = 0;
    struct message message;
    char data[MESSAGE_SIZE];

    request_send(sector, nblocks);

    while (channel_poll(EVENT_DATA, &message, data))
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

    request_readblocks(block, 1024, option_getdecimal("partoffset") + 2, 2);
    buffer_copy(sb, block, sizeof (struct ext2_superblock));

}

static void readblockgroup(struct ext2_blockgroup *bg, struct ext2_superblock *sb, unsigned int blocksize, unsigned int blockindex, unsigned int blockgroup)
{

    unsigned char block[4096];

    request_readblocks(block, 4096, option_getdecimal("partoffset") + blocksize, 8);
    buffer_copy(bg, block, sizeof (struct ext2_blockgroup));

}

static void readnode(struct ext2_node *node, struct ext2_superblock *sb, struct ext2_blockgroup *bg, unsigned int blocksize, unsigned int nodeindex)
{

    unsigned char block[4096];

    request_readblocks(block, 4096, option_getdecimal("partoffset") + blocksize * bg->blockTableAddress, 8);
    buffer_copy(node, block + nodeindex * sb->nodeSize, sizeof (struct ext2_node));

}

static unsigned int readdir(struct ext2_entry *entry, char *name, struct ext2_node *node, unsigned int blocksize, unsigned int index)
{

    unsigned char block[4096];
    unsigned int offset = 12 + 12 + 20;

    request_readblocks(block, 4096, option_getdecimal("partoffset") + blocksize * node->pointer0, 8);
    buffer_copy(entry, block + offset, sizeof (struct ext2_entry));
    buffer_copy(name, block + offset + 8, entry->length);

    return entry->length;

}

static void readdata(void *data, unsigned int count, struct ext2_node *node, unsigned int blocksize)
{

    unsigned char block[4096];

    request_readblocks(block, 4096, option_getdecimal("partoffset") + blocksize * node->pointer0, 8);
    buffer_write(data, count, block, 4096, 0);

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
*/

static void printdir(struct ext2_entry *entry, char *name)
{

    unsigned int length = entry->length;

    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "Inode: %u\n", &entry->node);
    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "Size: %h\n", &entry->size);
    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "Length: 0x%H2c\n", &entry->length);
    channel_send_fmt2(CHANNEL_DEFAULT, EVENT_DATA, "Name: %w\n", name, &length);

}

static void showinode(struct ext2_superblock *sb, unsigned int inode)
{

    struct ext2_blockgroup bg;
    struct ext2_node node;
    unsigned int blocksize = (1024 << sb->blockSize);
    unsigned int blockgroup = (inode - 1) / sb->nodeCountGroup;
    unsigned int nodeindex = (inode - 1) % sb->nodeCountGroup;
    unsigned int blockindex = (inode * sb->nodeSize) / blocksize;

    readblockgroup(&bg, sb, blocksize / 512, blockindex, blockgroup);
    readnode(&node, sb, &bg, blocksize / 512, nodeindex);

    if ((node.type & 0xF000) == 0x4000)
    {

        struct ext2_entry entry;
        char name[1024];

        readdir(&entry, name, &node, blocksize / 512, 0);
        printdir(&entry, name);

    }

    if ((node.type & 0xF000) == 0x8000)
    {

        char data[1024];
        unsigned int length = 4;

        readdata(data, 1024, &node, blocksize / 512);
        channel_send_fmt2(CHANNEL_DEFAULT, EVENT_DATA, "Data: %w\n", data, &length);

    }

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct ext2_superblock sb;

    call_walk_absolute(FILE_G5, option_getstring("volume"));
    call_link(FILE_G5, 8000);
    readsuperblock(&sb);

    if (isvalid(&sb))
    {

        showinode(&sb, 2);
        showinode(&sb, 12);

    }

    call_unlink(FILE_G5);
    channel_close();

}

void init(void)
{

    option_add("volume", "system:block/if:0/data");
    option_add("partoffset", "2048");
    channel_bind(EVENT_MAIN, onmain);

}

