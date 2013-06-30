#include <fudge.h>
#include <format/cpio.h>

void main()
{

    unsigned int offset = 0;

    do
    {

        struct cpio_header header;
        unsigned char name[512];

        call_read(CALL_DI, offset, sizeof (struct cpio_header), &header);

        if (!cpio_validate(&header))
            break;

        call_read(CALL_DI, offset + sizeof (struct cpio_header), header.namesize, name);
        call_write(CALL_DO, 0, header.namesize, name);
        call_write(CALL_DO, 0, 1, "\n");

        offset = cpio_next(&header, offset);

    } while (1);

}

