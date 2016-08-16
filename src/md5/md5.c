#include <abi.h>
#include <fudge.h>
#include <format/md5.h>

void main(void)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;
    unsigned char digest[16];
    unsigned int i;
    struct md5 s;

    md5_init(&s);
    file_open(CALL_PI);

    while ((count = file_read(CALL_PI, buffer, FUDGE_BSIZE)))
        md5_read(&s, buffer, count);

    file_close(CALL_PI);
    md5_write(&s, digest);
    file_open(CALL_PO);

    for (i = 0; i < 16; i++)
    {

        char num[FUDGE_NSIZE];

        file_writeall(CALL_PO, num, ascii_wzerovalue(num, FUDGE_NSIZE, digest[i], 16, 2, 0));

    }

    file_close(CALL_PO);

}

