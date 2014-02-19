#include <fudge/memory.h>
#include "cpio.h"

unsigned int cpio_validate(struct cpio_header *header)
{

    return header->magic == 0x71C7;

}

unsigned int cpio_next(struct cpio_header *header, unsigned int offset)
{

    unsigned int size = ((header->filesize[0] << 16) | header->filesize[1]);

    return offset + sizeof (struct cpio_header) + header->namesize + (header->namesize & 1) + size + (size & 1);

}

