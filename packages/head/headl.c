#include <abi.h>
#include <fudge.h>

void main()
{

    char num[32];
    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count0, count1, roff0, roff1, woff0 = 0;
    unsigned int lines = 0;
    unsigned int total;

    call_open(CALL_P1);

    count0 = call_read(CALL_P1, 0, 1, 32, num);

    call_close(CALL_P1);

    if (count0 == 32)
        return;

    total = ascii_rvalue(num, count0, 10);

    call_open(CALL_PO);
    call_open(CALL_P0);

    for (roff0 = 0; (count0 = call_read(CALL_P0, roff0, 1, FUDGE_BSIZE, buffer)); roff0 += roff1)
    {

        for (roff1 = 0; (count1 = memory_findbyte(buffer + roff1, count0 - roff1, '\n')); roff1 += count1)
        {

            if (++lines <= total)
                woff0 += call_write(CALL_PO, woff0, 1, count1, buffer + roff1);

        }

    }

    call_close(CALL_P0);
    call_close(CALL_PO);

}

