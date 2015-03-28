#include <abi.h>
#include <fudge.h>

void main()
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int offset, count;

    call_open(CALL_P0);

    for (offset = 0; (count = call_read(CALL_P0, offset, 1, FUDGE_BSIZE, buffer)); offset += count);

    call_close(CALL_P0);
    call_open(CALL_PO);
    call_write(CALL_PO, 0, 1, ascii_fromint(buffer, FUDGE_BSIZE, offset + count, 10), buffer);
    call_close(CALL_PO);

}

