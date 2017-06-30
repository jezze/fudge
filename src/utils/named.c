#include <abi.h>
#include <fudge.h>

void main(void)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;

    file_open(FILE_PI);

    count = file_read(FILE_PI, buffer, FUDGE_BSIZE);

    file_close(FILE_PI);

    while (--count && buffer[count - 1] != '/');

    file_open(FILE_PO);
    file_writeall(FILE_PO, buffer, count);
    file_close(FILE_PO);

}

