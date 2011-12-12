#ifndef MODULES_EXT2_H
#define MODULES_EXT2_H

#define EXT2_STATE_CLEAN 1
#define EXT2_STATE_DIRTY 2

#define EXT2_ERROR_IGNORE 1
#define EXT2_ERROR_RO     2
#define EXT2_ERROR_PANIC  3

#define EXT2_OSID_LINUX    0
#define EXT2_OSID_HURD     1
#define EXT2_OSID_MASIX    2
#define EXT2_OSID_FREEBSD  3
#define EXT2_OSID_OTHERBSD 4

#define EXT2_DRIVER_TYPE 320

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

struct ext2_driver
{

    struct modules_driver base;

};

extern void ext2_driver_init(struct ext2_driver *driver);

#endif

