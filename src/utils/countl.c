#include <abi.h>
#include <fudge.h>

void main(void)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count, lines = 0;

    call_open(CALL_P0);

    while ((count = call_read(CALL_P0, FUDGE_BSIZE, buffer)))
    {

        unsigned int i;

        for (i = 0; i < count; i++)
        {

            if (buffer[i] == '\n')
                lines++;

        }

    }

    call_close(CALL_P0);
    call_open(CALL_PO);
    call_write(CALL_PO, ascii_fromint(buffer, FUDGE_BSIZE, lines, 10), buffer);
    call_close(CALL_PO);

}

