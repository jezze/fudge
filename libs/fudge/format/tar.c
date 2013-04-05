#include "../string.h"
#include "tar.h"

unsigned int tar_validate(struct tar_header *header)
{

    unsigned char *address = (unsigned char *)header;
    unsigned int checksum = string_number(header->checksum, 8);
    unsigned int i;

    for (i = 0; i < TAR_BLOCK_SIZE; i++)
        checksum -= (i >= 148 && i < 156) ? 32 : address[i];

    return !checksum;

}

