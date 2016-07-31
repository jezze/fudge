#include <abi.h>
#include <fudge.h>

void main(void)
{

    file_open(CALL_PO);
    file_writeall(CALL_PO, "Hello world!\n", 13);
    file_close(CALL_PO);

}

