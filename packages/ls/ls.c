#include <fudge.h>

void main()
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count, roff, woff = 0;

    for (roff = 0; (count = call_read(CALL_DW, roff, FUDGE_BSIZE, buffer)); roff += count)
        woff += call_write(CALL_O0, woff, count, buffer);

}

