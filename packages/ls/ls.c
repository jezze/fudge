#include <fudge.h>

void main()
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count, roff, woff = 0;

    call_open(CALL_O0);
    call_open(CALL_DW);

    for (roff = 0; (count = call_read(CALL_DW, roff, FUDGE_BSIZE, buffer)); roff += count)
        woff += call_write(CALL_O0, woff, count, buffer);

    call_close(CALL_DW);
    call_close(CALL_O0);

}

