#include <abi.h>
#include <fudge.h>

void main(void)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int total, count;

    file_open(CALL_PI);

    for (total = 0; (count = file_read(CALL_PI, buffer, FUDGE_BSIZE)); total += count);

    file_close(CALL_PI);
    file_open(CALL_PO);
    file_writeall(CALL_PO, buffer, ascii_fromint(buffer, FUDGE_BSIZE, total + count, 10));
    file_close(CALL_PO);

}

