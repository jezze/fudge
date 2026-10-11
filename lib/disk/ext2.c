#include "ext2.h"

unsigned int ext2_validate(struct ext2_superblock *superblock)
{

    if (superblock->signature != 0xEF53)
        return 0;

    if (superblock->optionalAttributes & EXT2_FEATURE_JOURNAL)
        return 0;

    return superblock->requiredFeatures == EXT2_FEATURE_FILETYPE;

}

