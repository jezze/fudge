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
    unsigned int count, roff, loff;
    unsigned int lines = 0;

    for (roff = 0; (count = call_read(CALL_DI, roff, FUDGE_BSIZE, buffer)); roff += loff)
    {

        unsigned int count2;

        for (loff = 0; (count2 = nextline(loff, count, buffer)); loff += count2)
            lines++;

        if (!loff)
            return;

    }

    call_write(CALL_DO, 0, memory_write_number(buffer, FUDGE_BSIZE, lines, 10, 0), buffer);

}

