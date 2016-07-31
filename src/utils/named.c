#include <abi.h>
#include <fudge.h>

void main(void)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;

    file_open(CALL_PI);

    count = file_read(CALL_PI, buffer, FUDGE_BSIZE);

    file_close(CALL_PI);

    while (--count && buffer[count - 1] != '/');

    file_open(CALL_PO);
    file_writeall(CALL_PO, buffer, count);
    file_close(CALL_PO);

}

