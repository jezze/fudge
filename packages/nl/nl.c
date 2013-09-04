#include <arch.h>
#include <fudge.h>

static unsigned int nextline(unsigned int start, unsigned int count, unsigned char *buffer)
{

    unsigned int i;

    for (i = start; i < count; i++)
    {

        if (buffer[i] == '\n')
            return i + 1 - start;

    }

    return 0;

}


void main()
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;
    unsigned int offset;
    unsigned int offset2;
    unsigned int last = 0;
    unsigned int lines = 0;

    for (offset = 0; (count = call_read(CALL_DI, offset, FUDGE_BSIZE, buffer)); offset += offset2)
    {

        unsigned int count2;

        for (offset2 = 0; (count2 = nextline(offset2, count, buffer)); offset2 += count2)
        {

            unsigned char num[32];

            last += call_write(CALL_DO, last, memory_write_number(num, 32, lines, 10, 0), num);
            last += call_write(CALL_DO, last, 2, ": ");
            last += call_write(CALL_DO, last, count2, buffer + offset2);
            lines++;

        }

        if (!offset2)
            return;

    }

}

