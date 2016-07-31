#include <abi.h>
#include <fudge.h>

void main(void)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;
    unsigned int offset;

    file_open(CALL_PI);

    count = file_read(CALL_PI, buffer, FUDGE_BSIZE);

    file_close(CALL_PI);

    offset = count;

    while (--offset && buffer[offset - 1] != '/');

    file_open(CALL_PO);
    file_writeall(CALL_PO, buffer + offset, count - offset);
    file_close(CALL_PO);

}

