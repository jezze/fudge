#include "../string.h"
#include "tar.h"

unsigned int tar_validate(unsigned int count, void *buffer)
{

    struct tar_header *header = buffer;
    unsigned int checksum = string_number(header->checksum, 8);
    unsigned char *address = buffer;
    unsigned int i;

    if (count < TAR_BLOCK_SIZE)
        return 0;

    for (i = 0; i < TAR_BLOCK_SIZE; i++)
        checksum -= (i >= 148 && i < 156) ? 32 : address[i];

    return !checksum;

}

unsigned int tar_next(struct tar_header *header, unsigned int offset)
{

    unsigned int size = string_number(header->size, 8);

    return offset + ((size / TAR_BLOCK_SIZE) + ((size % TAR_BLOCK_SIZE) ? 2 : 1)) * TAR_BLOCK_SIZE;

}

