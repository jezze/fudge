#include <fudge.h>

void main()
{

    char num[32];
    char buffer[FUDGE_BSIZE];
    unsigned int count;
    unsigned int offset;
    unsigned int start;
    unsigned int o = 0;
    unsigned int lines = 0;

    for (offset = 0; (count = call_read(FUDGE_IN, offset, FUDGE_BSIZE, buffer)); offset += start)
    {

        unsigned int i;

        start = 0;

        for (i = 0; i < count; i++)
        {

            if (buffer[i] != '\n')
                continue;

            o += call_write(FUDGE_OUT, o, string_write_num(num, 32, lines, 10) - 1, num);
            o += call_write(FUDGE_OUT, o, 2, ": ");
            o += call_write(FUDGE_OUT, o, i - start + 1, buffer + start);

            lines++;
            start = i + 1;

        }

        if (!start)
            return;

    }

}

