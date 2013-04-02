#include <fudge.h>

void main()
{

    char buffer[FUDGE_BSIZE];
    unsigned int count;
    unsigned int offset;
    unsigned int start;

    for (offset = 0; (count = call_read(FUDGE_IN, offset, FUDGE_BSIZE, buffer)); offset += start)
    {

        unsigned int i;

        start = 0;

        for (i = 0; i < count; i++)
        {

            if (buffer[i] != '\n')
                continue;

            call_write(FUDGE_OUT, 0, 10, "00000000: ");
            call_write(FUDGE_OUT, 0, i - start + 1, buffer + start);

            start = i + 1;

        }

        if (!start)
            return;

    }

}

