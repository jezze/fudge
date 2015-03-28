#include <abi.h>
#include <fudge.h>

void main()
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;

    call_open(CALL_P0);

    count = call_read(CALL_P0, 0, 1, FUDGE_BSIZE, buffer);

    call_close(CALL_P0);

    while (--count && buffer[count - 1] != '/');

    call_open(CALL_PO);
    call_write(CALL_PO, 0, 1, count, buffer);
    call_close(CALL_PO);

}

