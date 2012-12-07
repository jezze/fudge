#include <fudge.h>

void main()
{

    char buffer[FUDGE_BSIZE];
    unsigned int count1;
    unsigned int count2;

    if (!call_open(3, 15, "/system/tty/cwd"))
        return;

    count1 = call_read(3, 0, FUDGE_BSIZE, buffer);
    count2 = call_read(FUDGE_IN, 0, FUDGE_BSIZE - count1, buffer + count1);

    if (!count1 || !count2)
        return;

    if (memory_match(buffer + count1, "/", 1))
        count1 = memory_write(buffer, FUDGE_BSIZE, buffer + count1, count2, 0);
    else
        count1 += count2;

    if (!call_open(4, count1, buffer))
        return;

    call_write(3, 0, count1, buffer);
    call_close(3);
    call_close(4);

}

