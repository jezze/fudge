#include "ext2.h"

unsigned int ext2_read16(void *data)
{

    unsigned char *bytes = data;

    return (bytes[1] << 8) | bytes[0];

}

unsigned int ext2_read32(void *data)
{

    unsigned char *bytes = data;

    return ((unsigned int)bytes[3] << 24) | (bytes[2] << 16) | (bytes[1] << 8) | bytes[0];

}

void ext2_write16(void *data, unsigned int value)
{

    unsigned char *bytes = data;

    bytes[0] = value;
    bytes[1] = value >> 8;

}

void ext2_write32(void *data, unsigned int value)
{

    unsigned char *bytes = data;

    bytes[0] = value;
    bytes[1] = value >> 8;
    bytes[2] = value >> 16;
    bytes[3] = value >> 24;

}

/* disk structures are little-endian: these swap fields on big-endian hosts, in either direction */
void ext2_convertsuperblock(struct ext2_superblock *superblock)
{

    superblock->nodeCount = ext2_read32(&superblock->nodeCount);
    superblock->blockCount = ext2_read32(&superblock->blockCount);
    superblock->blockCountSuper = ext2_read32(&superblock->blockCountSuper);
    superblock->blockCountUnalloc = ext2_read32(&superblock->blockCountUnalloc);
    superblock->nodeCountUnalloc = ext2_read32(&superblock->nodeCountUnalloc);
    superblock->superblockIndex = ext2_read32(&superblock->superblockIndex);
    superblock->blockSize = ext2_read32(&superblock->blockSize);
    superblock->fragmentSize = ext2_read32(&superblock->fragmentSize);
    superblock->blockCountGroup = ext2_read32(&superblock->blockCountGroup);
    superblock->fragmentCountGroup = ext2_read32(&superblock->fragmentCountGroup);
    superblock->nodeCountGroup = ext2_read32(&superblock->nodeCountGroup);
    superblock->lastMountTime = ext2_read32(&superblock->lastMountTime);
    superblock->lastWriteTime = ext2_read32(&superblock->lastWriteTime);
    superblock->currentCheckCount = ext2_read16(&superblock->currentCheckCount);
    superblock->maxCheckCount = ext2_read16(&superblock->maxCheckCount);
    superblock->signature = ext2_read16(&superblock->signature);
    superblock->state = ext2_read16(&superblock->state);
    superblock->error = ext2_read16(&superblock->error);
    superblock->minorVersion = ext2_read16(&superblock->minorVersion);
    superblock->lastCheckTime = ext2_read32(&superblock->lastCheckTime);
    superblock->forcedIntervalTime = ext2_read32(&superblock->forcedIntervalTime);
    superblock->osId = ext2_read32(&superblock->osId);
    superblock->majorVersion = ext2_read32(&superblock->majorVersion);
    superblock->userId = ext2_read16(&superblock->userId);
    superblock->groupId = ext2_read16(&superblock->groupId);
    superblock->firstUnreservedNode = ext2_read32(&superblock->firstUnreservedNode);
    superblock->nodeSize = ext2_read16(&superblock->nodeSize);
    superblock->superblockGroup = ext2_read16(&superblock->superblockGroup);
    superblock->optionalAttributes = ext2_read32(&superblock->optionalAttributes);
    superblock->requiredFeatures = ext2_read32(&superblock->requiredFeatures);
    superblock->unsupportedFeatures = ext2_read32(&superblock->unsupportedFeatures);
    superblock->algorithm = ext2_read32(&superblock->algorithm);
    superblock->unused = ext2_read16(&superblock->unused);
    superblock->journalNode = ext2_read32(&superblock->journalNode);
    superblock->journalDevice = ext2_read32(&superblock->journalDevice);
    superblock->firstOrphan = ext2_read32(&superblock->firstOrphan);

}

void ext2_convertblockgroup(struct ext2_blockgroup *blockgroup)
{

    blockgroup->blockUsageAddress = ext2_read32(&blockgroup->blockUsageAddress);
    blockgroup->nodeUsageAddress = ext2_read32(&blockgroup->nodeUsageAddress);
    blockgroup->blockTableAddress = ext2_read32(&blockgroup->blockTableAddress);
    blockgroup->blockCountUnalloc = ext2_read16(&blockgroup->blockCountUnalloc);
    blockgroup->nodeCountUnalloc = ext2_read16(&blockgroup->nodeCountUnalloc);
    blockgroup->directoryCount = ext2_read16(&blockgroup->directoryCount);

}

void ext2_convertnode(struct ext2_node *node)
{

    unsigned int i;

    node->type = ext2_read16(&node->type);
    node->userId = ext2_read16(&node->userId);
    node->sizeLow = ext2_read32(&node->sizeLow);
    node->lastAccessTime = ext2_read32(&node->lastAccessTime);
    node->creationTime = ext2_read32(&node->creationTime);
    node->lastModificationTime = ext2_read32(&node->lastModificationTime);
    node->deletionTime = ext2_read32(&node->deletionTime);
    node->groupId = ext2_read16(&node->groupId);
    node->hardCount = ext2_read16(&node->hardCount);
    node->sectorCount = ext2_read32(&node->sectorCount);
    node->flags = ext2_read32(&node->flags);
    node->osSpecific = ext2_read32(&node->osSpecific);

    for (i = 0; i < 12; i++)
        node->pointer[i] = ext2_read32(&node->pointer[i]);

    node->singlyIndirectPointer = ext2_read32(&node->singlyIndirectPointer);
    node->doublyIndirectPointer = ext2_read32(&node->doublyIndirectPointer);
    node->tripplyIndirectPointer = ext2_read32(&node->tripplyIndirectPointer);
    node->generation = ext2_read32(&node->generation);
    node->extendedAttributes = ext2_read32(&node->extendedAttributes);
    node->sizeHigh = ext2_read32(&node->sizeHigh);
    node->fragmentBlockAddress = ext2_read32(&node->fragmentBlockAddress);

}

unsigned int ext2_validate(struct ext2_superblock *superblock)
{

    if (ext2_read16(&superblock->signature) != 0xEF53)
        return 0;

    if (ext2_read32(&superblock->optionalAttributes) & EXT2_FEATURE_JOURNAL)
        return 0;

    return ext2_read32(&superblock->requiredFeatures) == EXT2_FEATURE_FILETYPE;

}

