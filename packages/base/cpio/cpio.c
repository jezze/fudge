#include <fudge.h>
#include <format/cpio.h>

void print_names(unsigned int offset)
{

    struct cpio_header header;
    unsigned char buffer[512];
    unsigned int count;

    do
    {

        call_read(CALL_DI, offset, sizeof (struct cpio_header), &header);

        if (!cpio_validate(&header))
            break;

        count = call_read(CALL_DI, offset + sizeof (struct cpio_header), header.namesize, buffer);
        call_write(CALL_DO, 0, count, buffer);
        call_write(CALL_DO, 0, 1, "\n");

    } while ((offset = cpio_next(&header, offset)));

}

void print_content(unsigned int offset)
{

    struct cpio_header header;
    unsigned char buffer[1024];
    unsigned int count;
    unsigned int size;

    call_read(CALL_DI, offset, sizeof (struct cpio_header), &header);

    if (!cpio_validate(&header))
        return;

    size = ((header.filesize[0] << 16) | header.filesize[1]);
    count = call_read(CALL_DI, offset + sizeof (struct cpio_header) + header.namesize + (header.namesize & 1), size, buffer);
    call_write(CALL_DO, 0, count, buffer);

}

void main()
{

    print_names(0);
    print_content(0);

}

