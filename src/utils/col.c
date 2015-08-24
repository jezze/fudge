#include <abi.h>
#include <fudge.h>

void main(void)
{
/*
    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count0, count1, roff0, roff1, woff = 0;

    call_open(CALL_PO);
    call_open(CALL_P0);

    for (roff0 = 0; (count0 = call_read(CALL_P0, roff0, 1, FUDGE_BSIZE, buffer)); roff0 += roff1)
    {

        for (roff1 = 0; (count1 = memory_findbyte(buffer + roff1, count0 - roff1, '\n')); roff1 += count1)
        {

            unsigned int length = (count1 > 14) ? 14 : count1 - 1;

            woff += call_write(CALL_PO, woff, length, 1, buffer + roff1);
            woff += call_write(CALL_PO, woff, 16 - length, 1, "                ");

        }

    }

    call_close(CALL_P0);
    call_close(CALL_PO);
*/
}

