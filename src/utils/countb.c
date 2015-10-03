#include <abi.h>
#include <fudge.h>

void main(void)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int total, count;

    call_open(CALL_P0);

    for (total = 0; (count = call_read(CALL_P0, FUDGE_BSIZE, buffer)); total += count);

    call_close(CALL_P0);
    call_open(CALL_PO);
    call_write(CALL_PO, ascii_fromint(buffer, FUDGE_BSIZE, total + count, 10), buffer);
    call_close(CALL_PO);

}

