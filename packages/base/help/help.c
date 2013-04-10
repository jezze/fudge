#include <fudge.h>

void main()
{

    char buffer[FUDGE_BSIZE];
    unsigned int offset;
    unsigned int count;

    if (!call_open(FUDGE_DI, FUDGE_DR, 13, "home/help.txt"))
        return;

    for (offset = 0; (count = call_read(FUDGE_DI, offset, FUDGE_BSIZE, buffer)); offset += count)
        call_write(FUDGE_DO, offset, count, buffer);

}

