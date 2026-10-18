#include "cpio.h"

static unsigned int readfield(char *field)
{

    unsigned int value = 0;
    unsigned int i;

    for (i = 0; i < CPIO_FIELDSIZE; i++)
    {

        char c = field[i];

        if (c >= 'a')
            value = value * 16 + c - 'a' + 10;
        else if (c >= 'A')
            value = value * 16 + c - 'A' + 10;
        else
            value = value * 16 + c - '0';

    }

    return value;

}

static unsigned int align(unsigned int value)
{

    return (value + 3) & ~3;

}

unsigned int cpio_validate(struct cpio_header *header)
{

    return header->magic[0] == '0' && header->magic[1] == '7' && header->magic[2] == '0' && header->magic[3] == '7' && header->magic[4] == '0' && header->magic[5] == '1';

}

unsigned int cpio_mode(struct cpio_header *header)
{

    return readfield(header->mode);

}

unsigned int cpio_namesize(struct cpio_header *header)
{

    return readfield(header->namesize);

}

unsigned int cpio_filedata(struct cpio_header *header)
{

    return align(sizeof (struct cpio_header) + cpio_namesize(header));

}

unsigned int cpio_filesize(struct cpio_header *header)
{

    return readfield(header->filesize);

}

unsigned int cpio_next(struct cpio_header *header)
{

    return cpio_filedata(header) + align(cpio_filesize(header));

}
