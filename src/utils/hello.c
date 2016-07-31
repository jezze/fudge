#include <abi.h>
#include <fudge.h>

void main(void)
{

    call_open(CALL_PO);
    file_writeall(CALL_PO, "Hello world!\n", 13);
    call_close(CALL_PO);

}

