#include <fudge.h>

void main()
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count, roff, loff, woff = 0;
    unsigned char kbuffer[FUDGE_BSIZE];
    unsigned int kcount = call_read(CALL_I1, 0, FUDGE_BSIZE, kbuffer);

    for (roff = 0; (count = call_read(CALL_I0, roff, FUDGE_BSIZE, buffer)); roff += loff)
    {

        unsigned int count2;

        for (loff = 0; (count2 = memory_findbyte(buffer + loff, count - loff, '\n')); loff += count2 + 1)
        {

            if (count2 < kcount)
                continue;

            if (!memory_match(buffer + loff, kbuffer, kcount))
                continue;

            woff += call_write(CALL_O0, woff, count2, buffer + loff);
            woff += call_write(CALL_O0, woff, 1, "\n");

        }

        if (!loff)
            break;

    }

}

