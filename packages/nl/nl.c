#include <fudge.h>

static unsigned int nextline(unsigned int offset, unsigned int count, char *buffer)
{

    unsigned int i;

    for (i = offset; i < count; i++)
    {

        if (buffer[i] == '\n')
            return i - offset + 1;

    }

    return 0;

}

void main()
{

    char buffer[FUDGE_BSIZE];
    unsigned int count, roff, loff, woff = 0;
    unsigned int lines = 0;

    for (roff = 0; (count = call_read(CALL_I0, roff, FUDGE_BSIZE, buffer)); roff += loff)
    {

        unsigned int count2;

        for (loff = 0; (count2 = nextline(loff, count, buffer)); loff += count2)
        {

            char num[32];

            woff += call_write(CALL_O0, woff, ascii_fromint(num, 32, lines++, 10), num);
            woff += call_write(CALL_O0, woff, 2, ": ");
            woff += call_write(CALL_O0, woff, count2, buffer + loff);

        }

        if (!loff)
            return;

    }

}

