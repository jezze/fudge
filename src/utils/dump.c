#include <abi.h>
#include <fudge.h>

void main(void)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;

    file_open(CALL_PO);
    file_open(CALL_PI);

    while ((count = file_read(CALL_PI, buffer, FUDGE_BSIZE)))
    {

        unsigned int i;

        for (i = 0; i < count; i++)
        {

            unsigned char num[FUDGE_NSIZE];

            file_writeall(CALL_PO, num, ascii_wzerovalue(num, FUDGE_NSIZE, buffer[i], 16, 2, 0));
            file_writeall(CALL_PO, "  ", 2);

        }

    }

    file_close(CALL_PI);
    file_close(CALL_PO);

}

