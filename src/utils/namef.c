#include <abi.h>
#include <fudge.h>

void main(void)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;
    unsigned int offset;

    call_open(CALL_PI);

    count = call_read(CALL_PI, FUDGE_BSIZE, buffer);

    call_close(CALL_PI);

    offset = count;

    while (--offset && buffer[offset - 1] != '/');

    call_open(CALL_PO);
    call_write(CALL_PO, count - offset, buffer + offset);
    call_close(CALL_PO);

}

