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

static unsigned char block[1024];

static void request_send(unsigned int sector, unsigned int count)
{

    struct {struct message_header header; struct event_blockrequest blockrequest;} message;

    message.blockrequest.sector = sector;
    message.blockrequest.count = count;

    message_initheader(&message.header, EVENT_BLOCKREQUEST, sizeof (struct event_blockrequest));
    file_writeall(FILE_G5, &message, message.header.length);

}

static unsigned int isvalid(struct ext2_superblock *superblock)
{

    return superblock->signature == 0xEF53;

}

static void print(struct ext2_superblock *superblock)
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

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct message message;

    file_walk2(FILE_G5, option_getstring("volume"));
    file_link(FILE_G5);
    request_send(option_getdecimal("partoffset") + 2, 1);

    if (channel_kpollevent(EVENT_DATA, &message))
    {

        struct ext2_superblock *superblock = (struct ext2_superblock *)block;

        buffer_write(block, 1024, message.data.buffer, message_datasize(&message.header), 0);

        if (isvalid(superblock))
            print(superblock);

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

