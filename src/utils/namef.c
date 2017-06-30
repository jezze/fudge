#include <abi.h>
#include <fudge.h>

void main(void)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;
    unsigned int offset;

    file_open(FILE_PI);

    count = file_read(FILE_PI, buffer, FUDGE_BSIZE);

    file_close(FILE_PI);

    offset = count;

    while (--offset && buffer[offset - 1] != '/');

    file_open(FILE_PO);
    file_writeall(FILE_PO, buffer + offset, count - offset);
    file_close(FILE_PO);

}

