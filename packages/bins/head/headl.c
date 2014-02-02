#include <fudge.h>

void main()
{

    char num[32];
    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count, roff, loff, woff0 = 0, woff1 = 0;
    unsigned int lines = 0;
    unsigned int total;

    memory_clear(num, 32);

    count = call_read(CALL_I1, 0, 32, num);

    if (count == 32)
        return;

    total = ascii_read_value(num, count, 10);

    for (roff = 0; (count = call_read(CALL_I0, roff, FUDGE_BSIZE, buffer)); roff += loff)
    {

        unsigned int count2;

        for (loff = 0; (count2 = memory_findbyte(buffer + loff, count - loff, '\n')); loff += count2 + 1)
        {

            if (++lines <= total)
            {

                woff0 += call_write(CALL_O0, woff0, count2, buffer + loff);
                woff0 += call_write(CALL_O0, woff0, 1, "\n");

            }

            else
            {

                woff1 += call_write(CALL_O1, woff1, count2, buffer + loff);
                woff1 += call_write(CALL_O1, woff1, 1, "\n");

            }

        }

        if (!loff)
            return;

    }

}

