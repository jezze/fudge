#include <fudge.h>

void main()
{

    char buffer[FUDGE_BSIZE];
    unsigned int offset;
    unsigned int count;

    if (!call_open(CALL_D0, CALL_DR, 13, "home/help.txt"))
        return;

    for (offset = 0; (count = call_read(CALL_D0, offset, FUDGE_BSIZE, buffer)); offset += count)
        call_write(CALL_DO, offset, count, buffer);

}

