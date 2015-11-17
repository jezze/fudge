#include <abi.h>
#include <fudge.h>
#include <lib/file.h>

void main(void)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;

    call_open(CALL_PO);
    call_open(CALL_PI);

    while ((count = file_read(CALL_PI, buffer, FUDGE_BSIZE)))
    {

        unsigned int i;

        for (i = 0; i < count; i++)
        {

            unsigned char num[32];

            file_writeall(CALL_PO, num, ascii_wzerovalue(num, 32, buffer[i], 16, 2, 0));
            file_writeall(CALL_PO, "  ", 2);

        }

    }

    call_close(CALL_PI);
    call_close(CALL_PO);

}

