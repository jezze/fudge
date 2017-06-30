#include <abi.h>
#include <fudge.h>

void main(void)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;

    file_open(FILE_PO);
    file_open(FILE_PI);

    while ((count = file_read(FILE_PI, buffer, FUDGE_BSIZE)))
        file_writeall(FILE_PO, buffer, count);

    file_close(FILE_PI);
    file_close(FILE_PO);

}

