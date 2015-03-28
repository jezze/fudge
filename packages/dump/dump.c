#include <abi.h>
#include <fudge.h>

void main()
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count, roff, woff = 0;

    call_open(CALL_PO);
    call_open(CALL_P0);

    for (roff = 0; (count = call_read(CALL_P0, roff, 1, FUDGE_BSIZE, buffer)); roff += count)
    {

        unsigned int i;

        for (i = 0; i < count; i++)
        {

            unsigned char num[32];

            woff += call_write(CALL_PO, woff, 1, ascii_wzerovalue(num, 32, buffer[i], 16, 2, 0), num);
            woff += call_write(CALL_PO, woff, 1, 2, "  ");

        }

    }

    call_close(CALL_P0);
    call_close(CALL_PO);

}

