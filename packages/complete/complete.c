#include <fudge.h>

static unsigned int nextline(unsigned int offset, unsigned int count, unsigned char *buffer)
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

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count, roff, loff, woff = 0;
    unsigned char kbuffer[FUDGE_BSIZE];
    unsigned int kcount = call_read(CALL_I1, 0, FUDGE_BSIZE, kbuffer);

    for (roff = 0; (count = call_read(CALL_I0, roff, FUDGE_BSIZE, buffer)); roff += loff)
    {

        unsigned int count2;

        for (loff = 0; (count2 = nextline(loff, count, buffer)); loff += count2)
        {

            if (count2 < kcount)
                continue;

            if (!memory_match(buffer + loff, kbuffer, kcount))
                continue;

            woff += call_write(CALL_O0, woff, count2, buffer + loff);

        }

        if (!loff)
            return;

    }

}

