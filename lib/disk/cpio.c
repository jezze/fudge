#include "cpio.h"

unsigned int cpio_validate(struct cpio_header *header)
{

    return header->magic == 0x71C7;

}

unsigned int cpio_filedata(struct cpio_header *header)
{

    return sizeof (struct cpio_header) + header->namesize + (header->namesize & 1);

}

unsigned int cpio_filesize(struct cpio_header *header)
{

    return (header->filesize[0] << 16) | header->filesize[1];

}

unsigned int cpio_next(struct cpio_header *header)
{

    unsigned int size = cpio_filesize(header);

    return cpio_filedata(header) + size + (size & 1);

}

