#include <fudge.h>

void main()
{

    char buffer[0x1000];
    unsigned int offset;
    unsigned int count;

    count = call_read(FILE_STDIN, 0, 0x1000, buffer);

    if (!count)
    {

        if (!call_open(3, 8, "/tty/cwd"))
            return;

        count = call_read(3, 0, 0x1000, buffer);

    }

    if (!call_open(3, count, buffer))
        return;

    for (offset = 0; (count = call_read(3, offset, 0x1000, buffer)); offset += 0x1000)
        call_write(FILE_STDOUT, offset, count, buffer);

    call_close(3);

}

