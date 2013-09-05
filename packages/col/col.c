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
    unsigned int count;
    unsigned int offset;
    unsigned int offset2;
    unsigned int last = 0;

    for (offset = 0; (count = call_read(CALL_DI, offset, FUDGE_BSIZE, buffer)); offset += offset2)
    {

        unsigned int count2;

        for (offset2 = 0; (count2 = nextline(offset2, count, buffer)); offset2 += count2)
        {

            unsigned int length = (count2 > 14) ? 14 : count2 - 1;

            last += call_write(CALL_DO, last, length, buffer + offset2);
            last += call_write(CALL_DO, last, 16 - length, "                ");

        }

        if (!offset2)
            return;

    }

    call_write(CALL_DO, last, 1, "\n");

}

