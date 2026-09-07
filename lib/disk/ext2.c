#include "ext2.h"

unsigned int ext2_validate(struct ext2_superblock *superblock)
{

    return superblock->signature == 0xEF53;

}

