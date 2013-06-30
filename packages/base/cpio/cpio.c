#include <fudge.h>
#include <format/cpio.h>

unsigned int next(struct cpio_header *header)
{

    unsigned int size = ((header->filesize[0] << 16) | header->filesize[1]);
    unsigned int offset = sizeof (struct cpio_header) + header->namesize + size;

    if (header->namesize & 1)
        offset++;

    if (size & 1)
        offset++;

    return offset;

}

void write(struct cpio_header *header, unsigned int offset)
{

    unsigned char name[512];

    call_read(CALL_DI, offset + sizeof (struct cpio_header), header->namesize, name);
    call_write(CALL_DO, 0, header->namesize, name);
    call_write(CALL_DO, 0, 1, "\n");

}

void main()
{

    struct cpio_header header;
    unsigned int offset = 0;

    do
    {

        call_read(CALL_DI, offset, sizeof (struct cpio_header), &header);

        if (!cpio_validate(&header))
            break;

        write(&header, offset);

        offset += next(&header);

    } while (1);

}

