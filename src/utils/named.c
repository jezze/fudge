#include <abi.h>
#include <fudge.h>

void main(void)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;

    call_open(CALL_PI);

    count = call_read(CALL_PI, FUDGE_BSIZE, buffer);

    call_close(CALL_PI);

    while (--count && buffer[count - 1] != '/');

    call_open(CALL_PO);
    call_write(CALL_PO, count, buffer);
    call_close(CALL_PO);

}

