#include <abi.h>
#include <fudge.h>

void main(void)
{

    call_open(CALL_PO);
    call_write(CALL_PO, 13, "Hello world!\n");
    call_close(CALL_PO);

}

