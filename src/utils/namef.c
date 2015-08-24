#include <abi.h>
#include <fudge.h>

void main(void)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;
    unsigned int offset;

    call_open(CALL_P0);

    count = call_read(CALL_P0, FUDGE_BSIZE, buffer);

    call_close(CALL_P0);

    offset = count;

    while (--offset && buffer[offset - 1] != '/');

    call_open(CALL_PO);
    call_write(CALL_PO, count - offset, buffer + offset);
    call_close(CALL_PO);

}

