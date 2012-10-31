#include <fudge.h>

void main()
{

    char buffer[0x1000];
    unsigned int count1;
    unsigned int count2;

    if (!call_open(3, 8, "/tty/cwd"))
        return;

    count1 = call_read(3, 0, 0x1000, buffer);
    count2 = call_read(FILE_STDIN, 0, 0x1000 - count1, buffer + count1);

    if (!count1 || !count2)
        return;

    if (memory_match(buffer + count1, "/", 1))
        count1 = memory_write(buffer, 0x1000, buffer + count1, count2, 0);
    else
        count1 += count2;

    if (!call_open(4, count1, buffer))
        return;

    call_write(3, 0, count1, buffer);
    call_close(3);
    call_close(4);

}

