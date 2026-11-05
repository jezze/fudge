#include <fudge.h>
#include "tar.h"

unsigned int tar_readvalue(char *attribute, unsigned int size)
{

    unsigned int value = 0;
    unsigned int i = 0;

    while (i < size && attribute[i] == ' ')
        i++;

    for (; i < size && attribute[i] >= '0' && attribute[i] <= '7'; i++)
        value = value * 8 + attribute[i] - '0';

    return value;

}

unsigned int tar_validate(struct tar_header *header)
{

    unsigned int checksum = tar_readvalue(header->checksum, sizeof (header->checksum));
    unsigned char *address = (unsigned char *)header;
    unsigned int i = 0;

    for (; i < 148; i++)
        checksum -= address[i];

    for (; i < 156; i++)
        checksum -= 32;

    for (; i < TAR_BLOCK_SIZE; i++)
        checksum -= address[i];

    return !checksum;

}

unsigned int tar_next(struct tar_header *header, unsigned int offset)
{

    unsigned int size = tar_readvalue(header->size, sizeof (header->size));

    return offset + ((size / TAR_BLOCK_SIZE) + ((size % TAR_BLOCK_SIZE) ? 2 : 1)) * TAR_BLOCK_SIZE;

}

