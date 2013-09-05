#include <fudge.h>

void main()
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;
    unsigned int offset;
    unsigned int last = 0;

    for (offset = 0; (count = call_read(CALL_DI, offset, FUDGE_BSIZE, buffer)); offset += count)
        last += call_write(CALL_DO, last, count, buffer);

    for (offset = 0; (count = call_read(CALL_DC, offset, FUDGE_BSIZE, buffer)); offset += count)
        last += call_write(CALL_DO, last, count, buffer);

}

