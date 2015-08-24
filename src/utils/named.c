#include <abi.h>
#include <fudge.h>

void main(void)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;

    call_open(CALL_P0);

    count = call_read(CALL_P0, FUDGE_BSIZE, buffer);

    call_close(CALL_P0);

    while (--count && buffer[count - 1] != '/');

    call_open(CALL_PO);
    call_write(CALL_PO, count, buffer);
    call_close(CALL_PO);

}

