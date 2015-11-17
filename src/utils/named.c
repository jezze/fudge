#include <abi.h>
#include <fudge.h>
#include <lib/file.h>

void main(void)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;

    call_open(CALL_PI);

    count = file_read(CALL_PI, buffer, FUDGE_BSIZE);

    call_close(CALL_PI);

    while (--count && buffer[count - 1] != '/');

    call_open(CALL_PO);
    file_writeall(CALL_PO, buffer, count);
    call_close(CALL_PO);

}

