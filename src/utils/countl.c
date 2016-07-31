#include <abi.h>
#include <fudge.h>

void main(void)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count, lines = 0;

    call_open(CALL_PI);

    while ((count = file_read(CALL_PI, buffer, FUDGE_BSIZE)))
    {

        unsigned int i;

        for (i = 0; i < count; i++)
        {

            if (buffer[i] == '\n')
                lines++;

        }

    }

    call_close(CALL_PI);
    call_open(CALL_PO);
    file_writeall(CALL_PO, buffer, ascii_fromint(buffer, FUDGE_BSIZE, lines, 10));
    call_close(CALL_PO);

}

