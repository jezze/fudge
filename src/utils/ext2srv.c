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

};

struct ext2_blockgroup
{

    unsigned int blockUsageAddress;
    unsigned int nodeUsageAddress;
    unsigned int blockTableAddress;
    unsigned short blockCountUnalloc;
    unsigned short nodeCountUnalloc;
    unsigned short directoryCount;

};

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

};

struct ext2_entry
{

    unsigned int node;
    unsigned char size[2];
    unsigned char length;
    unsigned char type;

};

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

static void printsb(struct ext2_superblock *superblock)
{

    struct message message;

    message_init(&message, EVENT_DATA);
    message_putstring(&message, "Signature: 0x");
    message_putvalue(&message, superblock->signature, 16, 4);
    message_putstring(&message, "\n");
    message_putstring(&message, "NodeCount: ");
    message_putvalue(&message, superblock->nodeCount, 10, 0);
    message_putstring(&message, "\n");
    message_putstring(&message, "BlockSize: ");
    message_putvalue(&message, superblock->blockSize, 10, 0);
    message_putstring(&message, "\n");
    message_putstring(&message, "BlockCount: ");
    message_putvalue(&message, superblock->blockCount, 10, 0);
    message_putstring(&message, "\n");
    channel_sendmessage(&message);

}

static void printbg(struct ext2_blockgroup *blockgroup)
{

    struct message message;

    message_init(&message, EVENT_DATA);
    message_putstring(&message, "Inode table block: ");
    message_putvalue(&message, blockgroup->blockTableAddress, 10, 0);
    message_putstring(&message, "\n");
    message_putstring(&message, "Directory count: ");
    message_putvalue(&message, blockgroup->directoryCount, 10, 0);
    message_putstring(&message, "\n");
    channel_sendmessage(&message);

}

static void readsb(struct ext2_superblock *sb)
{

    unsigned char block[1024];

    request_readblocks(block, 1024, option_getdecimal("partoffset") + 2, 2);
    buffer_copy(sb, block, sizeof (struct ext2_superblock));

}

static void readbg(struct ext2_blockgroup *bgs, struct ext2_superblock *sb, unsigned int nbgs, unsigned int i)
{

    unsigned char block[1024];

    request_readblocks(block, 1024, option_getdecimal("partoffset") + 2 + sb->blockSize + i * sb->blockSize, sb->blockSize);
    buffer_copy(bgs, block, sizeof (struct ext2_blockgroup) * nbgs);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct ext2_superblock sb;

    file_walk2(FILE_G5, option_getstring("volume"));
    file_link(FILE_G5);
    readsb(&sb);

    if (isvalid(&sb))
    {

        struct ext2_blockgroup bgs[16];
        unsigned int nbgs = 16;
        unsigned int i;

        printsb(&sb);
        readbg(bgs, &sb, nbgs, 0);

        for (i = 0; i < nbgs; i++)
            printbg(&bgs[i]);

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

