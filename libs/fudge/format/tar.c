#include "../ascii.h"
#include "../memory.h"
#include "tar.h"

unsigned int tar_validate(void *buffer)
{

    struct tar_header *header = buffer;
    unsigned int checksum = ascii_read_value(header->checksum, memory_findbyte(header->checksum, 8, '\0'), 8);
    unsigned char *address = buffer;
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

    unsigned int size = ascii_read_value(header->size, memory_findbyte(header->size, 12, '\0'), 8);

    return offset + ((size / TAR_BLOCK_SIZE) + ((size % TAR_BLOCK_SIZE) ? 2 : 1)) * TAR_BLOCK_SIZE;

}

