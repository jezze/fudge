#include <abi.h>
#include <fudge.h>
#include <lib/file.h>

void main(void)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int total, count;

    call_open(CALL_PI);

    for (total = 0; (count = file_read(CALL_PI, buffer, FUDGE_BSIZE)); total += count);

    call_close(CALL_PI);
    call_open(CALL_PO);
    file_writeall(CALL_PO, buffer, ascii_fromint(buffer, FUDGE_BSIZE, total + count, 10));
    call_close(CALL_PO);

}

