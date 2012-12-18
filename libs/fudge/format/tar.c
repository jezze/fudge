#include "../string.h"
#include "tar.h"

unsigned int tar_validate(struct tar_header *header)
{

    unsigned int i;
    unsigned char *address = (unsigned char *)header;
    unsigned int checksum = string_read_num(header->checksum, 8);

    for (i = 0; i < TAR_BLOCK_SIZE; i++)
    {

        if (i >= 148 && i < 156)
            checksum -= 32;
        else
            checksum -= address[i];

    }

    return !checksum;

}

