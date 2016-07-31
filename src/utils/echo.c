#include <abi.h>
#include <fudge.h>

void main(void)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;

    file_open(CALL_PO);
    file_open(CALL_PI);

    while ((count = file_read(CALL_PI, buffer, FUDGE_BSIZE)))
        file_writeall(CALL_PO, buffer, count);

    file_close(CALL_PI);
    file_close(CALL_PO);

}

