#include <fudge.h>

void main()
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count, roff, loff, woff = 0;

    call_open(CALL_O0);
    call_open(CALL_I0);

    for (roff = 0; (count = call_read(CALL_I0, roff, FUDGE_BSIZE, buffer)); roff += loff)
    {

        unsigned int count2;

        for (loff = 0; (count2 = memory_findbyte(buffer + loff, count - loff, '\n')); loff += count2 + 1)
        {

            unsigned int length = (count2 > 14) ? 14 : count2;

            woff += call_write(CALL_O0, woff, length, buffer + loff);
            woff += call_write(CALL_O0, woff, 16 - length, "                ");

        }

        if (!loff)
            break;

    }

    call_close(CALL_I0);
    call_write(CALL_O0, woff, 1, "\n");
    call_close(CALL_O0);

}

