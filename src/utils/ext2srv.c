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
    unsigned int block = 1 + blockgroup / perblock;
    unsigned int offset = (blockgroup % perblock) * sizeof (struct ext2_blockgroup);
    unsigned char data[4096];

    read(data, 4096, block, blocksize);
    buffer_copy(bg, data + offset, sizeof (struct ext2_blockgroup));

}

static void readnode(struct ext2_node *node, unsigned int blocktable, unsigned int blocksize, unsigned int nodeindex, unsigned int nodesize)
{

    unsigned int perblock = blocksize / nodesize;
    unsigned int block = blocktable + nodeindex / perblock;
    unsigned int offset = (nodeindex % perblock) * nodesize;
    unsigned char data[4096];

    read(data, 4096, block, blocksize);
    buffer_copy(node, data + offset, sizeof (struct ext2_node));

}

/*
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

    readblockgroup(&bg, blocksize, blockindex, blockgroup);
    readnode(&node, bg.blockTableAddress, blocksize, nodeindex, sb->nodeSize);

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

static void simpleread(struct ext2_node *node, unsigned int id)
{

    unsigned int blocksize = (1024 << sb.blockSize);
    unsigned int blockgroup = (id - 1) / sb.nodeCountGroup;
    unsigned int nodeindex = (id - 1) % sb.nodeCountGroup;
    struct ext2_blockgroup bg;

    readblockgroup(&bg, blocksize, blockgroup);
    readnode(node, bg.blockTableAddress, blocksize, nodeindex, sb.nodeSize);

}

static unsigned int getindirect(unsigned int table_block, unsigned int index, unsigned int blocksize)
{

    unsigned int table[1024];   /* blocksize/4 pointers max, blocksize capped at 4096 */

    if (!table_block)
        return 0;

    read(table, blocksize, table_block, blocksize);

    return table[index];

}

static unsigned int getblock(struct ext2_node *node, unsigned int index, unsigned int blocksize)
{

    unsigned int ptrsperblock = blocksize / sizeof (unsigned int);

    if (index < 12)
    {

        switch (index)
        {

        case 0: return node->pointer0;
        case 1: return node->pointer1;
        case 2: return node->pointer2;
        case 3: return node->pointer3;
        case 4: return node->pointer4;
        case 5: return node->pointer5;
        case 6: return node->pointer6;
        case 7: return node->pointer7;
        case 8: return node->pointer8;
        case 9: return node->pointer9;
        case 10: return node->pointer10;
        case 11: return node->pointer11;

        }

    }

    index -= 12;

    if (index < ptrsperblock)
        return getindirect(node->singlyIndirectPointer, index, blocksize);

    index -= ptrsperblock;

    if (index < ptrsperblock * ptrsperblock)
    {

        unsigned int outer = index / ptrsperblock;
        unsigned int inner = index % ptrsperblock;

        return getindirect(getindirect(node->doublyIndirectPointer, outer, blocksize), inner, blocksize);

    }

    index -= ptrsperblock * ptrsperblock;

    {

        unsigned int outer = index / (ptrsperblock * ptrsperblock);
        unsigned int mid = (index / ptrsperblock) % ptrsperblock;
        unsigned int inner = index % ptrsperblock;
        unsigned int b1 = getindirect(node->tripplyIndirectPointer, outer, blocksize);
        unsigned int b2 = getindirect(b1, mid, blocksize);

        return getindirect(b2, inner, blocksize);

    }

}

static void onreadrequest(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned char data[MESSAGE_SIZE];
    struct event_readrequest *request = mdata;
    struct event_readresponse *response = (struct event_readresponse *)data;
    unsigned int blocksize = (1024 << sb.blockSize);
    unsigned char block[4096];
    struct ext2_node node;

    simpleread(&node, request->id);

    response->count = 0;

    switch (node.type & 0xF000)
    {

    case 0x4000:
        {

            unsigned int offset = request->offset;
            unsigned int capacity = MESSAGE_SIZE - sizeof (struct event_readresponse);
            struct record *records = (struct record *)(response + 1);
            unsigned int i = 0;
            unsigned int currentblock = 0xFFFFFFFF;

            while (offset < node.sizeLow && (i + 1) * sizeof (struct record) <= capacity)
            {

                unsigned int logicalblock = offset / blocksize;
                unsigned int blockoffset = offset % blocksize;
                struct ext2_entry *entry;

                if (logicalblock != currentblock)
                {

                    unsigned int physicalblock = getblock(&node, logicalblock, blocksize);

                    if (!physicalblock)
                        break;

                    read(block, 4096, physicalblock, blocksize);

                    currentblock = logicalblock;

                }

                entry = (struct ext2_entry *)(block + blockoffset);

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
        read(block, 4096, node.pointer0, blocksize);

        response->count = buffer_write(data + sizeof (struct event_readresponse), MESSAGE_SIZE - sizeof (struct event_readresponse), block, response->count, 0);

        channel_send(0, source, EVENT_READRESPONSE, sizeof (struct event_readresponse) + response->count, data);
 
        break;

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

        struct event_walkresponse response;

        response.id = id;

        channel_send(0, source, EVENT_WALKRESPONSE, sizeof (struct event_walkresponse), &response);

        return;

    }

    simpleread(&node, id);

    if ((node.type & 0xF000) == 0x4000)
    {

        unsigned int blocksize = (1024 << sb.blockSize);
        unsigned char block[4096];
        unsigned int offset = 0;

        read(block, 4096, node.pointer0, blocksize);

        while (offset < 4096)
        {

            struct ext2_entry *entry = (struct ext2_entry *)(block + offset);

            if (walkrequest->length == entry->length + 1 && buffer_match(entry + 1, path, entry->length))
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

