#include <fudge.h>

void main()
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count, roff, woff = 0;

    for (roff = 0; (count = call_read(CALL_DI, roff, FUDGE_BSIZE, buffer)); roff += count)
        woff += call_write(CALL_DO, woff, count, buffer);

    for (roff = 0; (count = call_read(CALL_DC, roff, FUDGE_BSIZE, buffer)); roff += count)
        woff += call_write(CALL_DO, woff, count, buffer);

}

