#include <abi.h>
#include <fudge.h>

void main(void)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int total, count;

    file_open(FILE_PI);

    for (total = 0; (count = file_read(FILE_PI, buffer, FUDGE_BSIZE)); total += count);

    file_close(FILE_PI);
    file_open(FILE_PO);
    file_writeall(FILE_PO, buffer, ascii_fromint(buffer, FUDGE_BSIZE, total + count, 10));
    file_close(FILE_PO);

}

