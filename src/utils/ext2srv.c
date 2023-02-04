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
    unsigned char size[2];
    unsigned char length;
    unsigned char type;

} __attribute__((packed));

static void request_send(unsigned int sector, unsigned int count)
{

    struct {struct message message; struct event_blockrequest blockrequest;} message;

    message.blockrequest.sector = sector;
    message.blockrequest.count = count;

    message_init(&message.message, EVENT_BLOCKREQUEST, sizeof (struct event_blockrequest));
    file_writeall(FILE_G5, &message, message.message.length);

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

static void printsuperblock(struct ext2_superblock *superblock)
{

    channel_sendfmt1(CHANNEL_DEFAULT, EVENT_DATA, "Signature: 0x%H4h\n", &superblock->signature);
    channel_sendfmt1(CHANNEL_DEFAULT, EVENT_DATA, "SuperblockIndex: %u\n", &superblock->superblockIndex);
    channel_sendfmt1(CHANNEL_DEFAULT, EVENT_DATA, "SuperblockGroup: %h\n", &superblock->superblockGroup);
    channel_sendfmt1(CHANNEL_DEFAULT, EVENT_DATA, "BlockSize: %u\n", &superblock->blockSize);
    channel_sendfmt1(CHANNEL_DEFAULT, EVENT_DATA, "BlockCount: %u\n", &superblock->blockCount);
    channel_sendfmt1(CHANNEL_DEFAULT, EVENT_DATA, "BlockCountSuper: %u\n", &superblock->blockCountSuper);
    channel_sendfmt1(CHANNEL_DEFAULT, EVENT_DATA, "NodeSize: %h\n", &superblock->nodeSize);
    channel_sendfmt1(CHANNEL_DEFAULT, EVENT_DATA, "NodeCount: %u\n", &superblock->nodeCount);
    channel_sendfmt1(CHANNEL_DEFAULT, EVENT_DATA, "NodeCountGroup: %u\n", &superblock->nodeCountGroup);
    channel_sendfmt1(CHANNEL_DEFAULT, EVENT_DATA, "FragmentSize: %u\n", &superblock->fragmentSize);
    channel_sendfmt1(CHANNEL_DEFAULT, EVENT_DATA, "FragmentCountGroup: %u\n", &superblock->fragmentCountGroup);

}

static void printblockgroup(struct ext2_blockgroup *blockgroup)
{

    channel_sendfmt1(CHANNEL_DEFAULT, EVENT_DATA, "Inode table block: %u\n", &blockgroup->blockTableAddress);
    channel_sendfmt1(CHANNEL_DEFAULT, EVENT_DATA, "Directory count: %h\n", &blockgroup->directoryCount);

}

static void printnode(struct ext2_node *node)
{

    channel_sendfmt1(CHANNEL_DEFAULT, EVENT_DATA, "Type: 0x%H4h\n", &node->type);
    channel_sendfmt1(CHANNEL_DEFAULT, EVENT_DATA, "Flags: 0x%H8u\n", &node->flags);
    channel_sendfmt1(CHANNEL_DEFAULT, EVENT_DATA, "User Id: %h\n", &node->userId);

}

static void readsuperblock(struct ext2_superblock *sb)
{

    unsigned char block[1024];

    request_readblocks(block, 1024, option_getdecimal("partoffset") + 2, 2);
    buffer_copy(sb, block, sizeof (struct ext2_superblock));

}

static void readblockgroups(struct ext2_blockgroup *bg, struct ext2_superblock *sb, unsigned int index, unsigned int offset)
{

    unsigned char block[1024];

    request_readblocks(block, 1024, option_getdecimal("partoffset") + 2 + offset * sb->blockSize, sb->blockSize);
    buffer_copy(bg, block + index * 32, sizeof (struct ext2_blockgroup));

}

static void readnode(struct ext2_node *node, struct ext2_superblock *sb, unsigned int index, unsigned int offset)
{

    unsigned char block[1024];

    request_readblocks(block, 1024, option_getdecimal("partoffset") + 2 + offset * sb->blockSize, sb->blockSize);
    buffer_copy(node, block + index * sb->nodeSize, sizeof (struct ext2_node));

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct ext2_superblock sb;
    unsigned int rootid = 2;

    file_walk2(FILE_G5, option_getstring("volume"));
    file_link(FILE_G5);
    readsuperblock(&sb);

    if (isvalid(&sb))
    {

        struct ext2_blockgroup bg;
        struct ext2_node node;
        unsigned int blockgroup = (rootid - 1) / sb.nodeCountGroup;
        unsigned int nodeindex = (rootid - 1) % sb.nodeCountGroup;
        unsigned int blockindex = (rootid * sb.nodeSize) / sb.blockSize;

        printsuperblock(&sb);
        channel_sendfmt1(CHANNEL_DEFAULT, EVENT_DATA, "BlockGroup: %u\n", &blockgroup);
        channel_sendfmt1(CHANNEL_DEFAULT, EVENT_DATA, "NodeIndex: %u\n", &nodeindex);
        channel_sendfmt1(CHANNEL_DEFAULT, EVENT_DATA, "BlockIndex: %u\n", &blockindex);
        readblockgroups(&bg, &sb, 0, 3 + blockgroup); /* add blockgroup here */
        printblockgroup(&bg);
        readnode(&node, &sb, nodeindex, bg.blockTableAddress + blockindex);
        printnode(&node);

    }

    file_unlink(FILE_G5);
    channel_close();

}

void init(void)
{

    option_add("volume", "system:block/if:0/data");
    option_add("partoffset", "2048");
    channel_bind(EVENT_MAIN, onmain);

}

