#include <abi.h>
#include <fudge.h>

void main()
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count, roff;

    call_open(CALL_P0);

    for (roff = 0; (count = call_read(CALL_P0, roff, 1, FUDGE_BSIZE, buffer)); roff += count);

    call_close(CALL_P0);
    call_open(CALL_PO);
    call_write(CALL_PO, 0, 1, ascii_fromint(buffer, FUDGE_BSIZE, roff + count, 10), buffer);
    call_close(CALL_PO);

}

