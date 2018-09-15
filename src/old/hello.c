#include <abi.h>
#include <fudge.h>

void main(void)
{

    file_open(FILE_PO);
    file_writeall(FILE_PO, "Hello world!\n", 13);
    file_close(FILE_PO);

}

