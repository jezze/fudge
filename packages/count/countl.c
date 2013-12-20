#include <fudge.h>

void main()
{

    char buffer[FUDGE_BSIZE];
    unsigned int count, roff, loff;
    unsigned int lines = 0;

    for (roff = 0; (count = call_read(CALL_I0, roff, FUDGE_BSIZE, buffer)); roff += loff)
    {

        unsigned int count2;

        for (loff = 0; (count2 = memory_findbyte(buffer + loff, count - loff, '\n')); loff += count2 + 1)
            lines++;

        if (!loff)
            return;

    }

    call_write(CALL_O0, 0, ascii_fromint(buffer, FUDGE_BSIZE, lines, 10), buffer);

}

