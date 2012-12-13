#include <fudge.h>

void main()
{

    char buffer[FUDGE_BSIZE];
    unsigned int offset;
    unsigned int count;

    count = call_read(FUDGE_IN, 0, FUDGE_BSIZE, buffer);

    if (!count)
    {

        if (!call_open(3, FUDGE_ROOT, 15, "/system/tty/cwd"))
            return;

        count = call_read(3, 0, FUDGE_BSIZE, buffer);

        call_close(3);

    }

    if (!call_open(FUDGE_IN, FUDGE_ROOT, count, buffer))
        return;

    for (offset = 0; (count = call_read(FUDGE_IN, offset, FUDGE_BSIZE, buffer)); offset += count)
        call_write(FUDGE_OUT, offset, count, buffer);

}

