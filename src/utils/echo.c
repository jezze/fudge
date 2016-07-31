#include <abi.h>
#include <fudge.h>

void main(void)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;

    call_open(CALL_PO);
    call_open(CALL_PI);

    while ((count = file_read(CALL_PI, buffer, FUDGE_BSIZE)))
        file_writeall(CALL_PO, buffer, count);

    call_close(CALL_PI);
    call_close(CALL_PO);

}

