#include <abi.h>
#include <fudge.h>

void main()
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count0, count1, roff0, roff1, woff = 0;

    call_open(CALL_O0);
    call_open(CALL_I0);

    for (roff0 = 0; (count0 = call_read(CALL_I0, roff0, FUDGE_BSIZE, buffer)); roff0 += roff1)
    {

        for (roff1 = 0; (count1 = memory_findbyte(buffer + roff1, count0 - roff1, '\n')); roff1 += count1)
        {

            unsigned int length = (count1 > 14) ? 14 : count1 - 1;

            woff += call_write(CALL_O0, woff, length, buffer + roff1);
            woff += call_write(CALL_O0, woff, 16 - length, "                ");

        }

    }

    call_close(CALL_I0);
    call_close(CALL_O0);

}

