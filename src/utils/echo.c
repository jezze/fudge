#include <abi.h>
#include <fudge.h>

void main(void)
{

    unsigned char buffer[FUDGE_BSIZE];

    call_open(CALL_PO);
    call_open(CALL_P0);

    while (call_write(CALL_PO, call_read(CALL_P0, FUDGE_BSIZE, buffer), buffer));

    call_close(CALL_P0);
    call_close(CALL_PO);

}

