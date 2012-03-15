#ifndef MODULES_EXT2_H
#define MODULES_EXT2_H

#define EXT2_STATE_CLEAN        1
#define EXT2_STATE_DIRTY        2

#define EXT2_ERROR_IGNORE       1
#define EXT2_ERROR_RO           2
#define EXT2_ERROR_PANIC        3

#define EXT2_OSID_LINUX         0
#define EXT2_OSID_HURD          1
#define EXT2_OSID_MASIX         2
#define EXT2_OSID_FREEBSD       3
#define EXT2_OSID_OTHERBSD      4

#define EXT2_NODE_TYPE_FIFO     0x1000
#define EXT2_NODE_TYPE_CHAR     0x2000
#define EXT2_NODE_TYPE_DIR      0x4000
#define EXT2_NODE_TYPE_BLOCK    0x6000
#define EXT2_NODE_TYPE_REGULAR  0x8000
#define EXT2_NODE_TYPE_SYM      0xA000
#define EXT2_NODE_TYPE_SOCKET   0xC000

#define EXT2_DRIVER_TYPE        0x0320

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

struct ext2_driver
{

    struct modules_driver base;
    struct ata_device *ataDevice;
    struct mbr_driver *mbrDriver;
    void (*read_blockgroup)(struct ext2_driver *self, unsigned int id, struct ext2_blockgroup *bg);
    void (*read_node)(struct ext2_driver *self, unsigned int id, struct ext2_blockgroup *bg, struct ext2_node *node);
    void (*read_content)(struct ext2_driver *self, struct ext2_node *node, void *buffer);

};

void ext2_driver_init(struct ext2_driver *driver);
void ext2_filesystem_init(struct modules_module *module);

#endif

