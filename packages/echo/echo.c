#include <abi.h>
#include <fudge.h>

void main()
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count, roff, woff = 0;

    call_open(CALL_PO);
    call_open(CALL_P0);

    for (roff = 0; (count = call_read(CALL_P0, roff, FUDGE_BSIZE, buffer)); roff += count)
        woff += call_write(CALL_PO, woff, count, buffer);

    call_close(CALL_P0);
    call_close(CALL_PO);

}

