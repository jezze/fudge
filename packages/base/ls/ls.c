#include <fudge.h>

void main()
{

    char buffer[FUDGE_BSIZE];
    unsigned int offset;
    unsigned int count;

    count = call_read(FUDGE_IN, 0, FUDGE_BSIZE, buffer);

    if (!count)
    {

        if (!call_open(3, 12, "/dev/tty/cwd"))
            return;

        count = call_read(3, 0, FUDGE_BSIZE, buffer);

    }

    if (!call_open(3, count, buffer))
        return;

    for (offset = 0; (count = call_read(3, offset, FUDGE_BSIZE, buffer)); offset += FUDGE_BSIZE)
        call_write(FUDGE_OUT, offset, count, buffer);

    call_close(3);

}

