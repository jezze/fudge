#include "../string.h"
#include "tar.h"

unsigned int tar_validate(unsigned int count, void *buffer)
{

    struct tar_header *header = buffer;
    unsigned char *address = buffer;
    unsigned int checksum;
    unsigned int i;

    if (count < TAR_BLOCK_SIZE)
        return 0;

    checksum = string_number(header->checksum, 8);

    for (i = 0; i < TAR_BLOCK_SIZE; i++)
        checksum -= (i >= 148 && i < 156) ? 32 : address[i];

    return !checksum;

}

