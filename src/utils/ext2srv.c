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

    struct {struct message_header header; struct event_blockrequest blockrequest;} message;

    message.blockrequest.sector = sector;
    message.blockrequest.count = count;

    message_initheader(&message.header, EVENT_BLOCKREQUEST, sizeof (struct event_blockrequest));
    file_writeall(FILE_G5, &message, message.header.length);

}

static void request_readblocks(void *buffer, unsigned int count, unsigned int sector, unsigned int nblocks)
{

    unsigned int total = nblocks * 512;
    unsigned int read = 0;
    struct message message;

    request_send(sector, nblocks);

    while (channel_kpollevent(EVENT_DATA, &message))
    {

        read += buffer_write(buffer, count, message.data.buffer, message_datasize(&message.header), read);

        if (read == total)
            break;

    }

}

static unsigned int isvalid(struct ext2_superblock *superblock)
{

    return superblock->signature == 0xEF53;

}

static void printvalue(char *name, int value, int base, int padding)
{

    struct message message;

    message_init(&message, EVENT_DATA);
    message_putstring(&message, name);
    message_putstring(&message, ": ");
    message_putvalue(&message, value, base, padding);
    message_putstring(&message, "\n");
    channel_sendmessage(&message);

}

static void printsuperblock(struct ext2_superblock *superblock)
{

    struct message message;

    message_init(&message, EVENT_DATA);
    message_putfmt1(&message, "Signature: 0x%H4h\n", &superblock->signature);
    message_putfmt1(&message, "SuperblockIndex: %u\n", &superblock->superblockIndex);
    message_putfmt1(&message, "SuperblockGroup: %h\n", &superblock->superblockGroup);
    message_putfmt1(&message, "BlockSize: %u\n", &superblock->blockSize);
    message_putfmt1(&message, "BlockCount: %u\n", &superblock->blockCount);
    message_putfmt1(&message, "BlockCountSuper: %u\n", &superblock->blockCountSuper);
    message_putfmt1(&message, "NodeSize: %h\n", &superblock->nodeSize);
    message_putfmt1(&message, "NodeCount: %u\n", &superblock->nodeCount);
    message_putfmt1(&message, "NodeCountGroup: %u\n", &superblock->nodeCountGroup);
    message_putfmt1(&message, "FragmentSize: %u\n", &superblock->fragmentSize);
    message_putfmt1(&message, "FragmentCountGroup: %u\n", &superblock->fragmentCountGroup);
    channel_sendmessage(&message);

}

static void printblockgroup(struct ext2_blockgroup *blockgroup)
{

    struct message message;

    message_init(&message, EVENT_DATA);
    message_putfmt1(&message, "Inode table block: %u\n", &blockgroup->blockTableAddress);
    message_putfmt1(&message, "Directory count: %h\n", &blockgroup->directoryCount);
    channel_sendmessage(&message);

}

static void printnode(struct ext2_node *node)
{

    struct message message;

    message_init(&message, EVENT_DATA);
    message_putfmt1(&message, "Type: 0x%H4h\n", &node->type);
    message_putfmt1(&message, "Flags: 0x%H8u\n", &node->flags);
    message_putfmt1(&message, "User Id: %h\n", &node->userId);
    channel_sendmessage(&message);

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

        printvalue("BlockGroup", blockgroup, 10, 0);
        printvalue("NodeIndex", nodeindex, 10, 0);
        printvalue("BlockIndex", blockindex, 10, 0);

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

