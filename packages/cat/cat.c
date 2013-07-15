#include <fudge.h>

void main()
{

    char buffer[FUDGE_BSIZE];
    unsigned int offset;
    unsigned int count;
    unsigned int last;

    for (offset = 0; (count = call_read(CALL_DI, offset, FUDGE_BSIZE, buffer)); offset += count)
        call_write(CALL_DO, offset, count, buffer);

    last = offset;

    for (offset = 0; (count = call_read(CALL_DC, offset, FUDGE_BSIZE, buffer)); offset += count)
        call_write(CALL_DO, last + offset, count, buffer);

}

