#include <fudge.h>

void main()
{

    char buffer[FUDGE_BSIZE];
    unsigned int count, roff, loff, woff = 0;
    unsigned int lines = 0;

    call_open(CALL_O0);
    call_open(CALL_I0);

    for (roff = 0; (count = call_read(CALL_I0, roff, FUDGE_BSIZE, buffer)); roff += loff)
    {

        unsigned int count2;

        for (loff = 0; (count2 = memory_findbyte(buffer + loff, count - loff, '\n')); loff += count2 + 1)
        {

            char num[32];

            woff += call_write(CALL_O0, woff, ascii_fromint(num, 32, lines++, 10), num);
            woff += call_write(CALL_O0, woff, 2, ": ");
            woff += call_write(CALL_O0, woff, count2, buffer + loff);
            woff += call_write(CALL_O0, woff, 1, "\n");

        }

        if (!loff)
            return;

    }

    call_close(CALL_I0);
    call_close(CALL_O0);

}

