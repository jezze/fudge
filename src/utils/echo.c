#include <abi.h>
#include <fudge.h>

void main(void)
{

    unsigned char buffer[FUDGE_BSIZE];

    call_open(CALL_PO);
    call_open(CALL_PI);

    while (call_write(CALL_PO, call_read(CALL_PI, FUDGE_BSIZE, buffer), buffer));

    call_close(CALL_PI);
    call_close(CALL_PO);

}

