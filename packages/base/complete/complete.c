#include <fudge.h>

void main()
{

    char buffer[FUDGE_BSIZE];
    char buffer2[FUDGE_BSIZE];
    unsigned int count;
    unsigned int count2;
    unsigned int i;
    unsigned int start = 0;

    count = call_read(FUDGE_IN, 0, FUDGE_BSIZE, buffer);
    count2 = call_read(FUDGE_CWD, 0, FUDGE_BSIZE, buffer2);

    for (i = 0; i < count2; i++)
    {

        if (buffer2[i] != '\n')
            continue;

        if (start)
        {

            call_write(FUDGE_OUT, 0, i - start + 1, buffer2 + start);
            start = 0;

        }

        if (memory_match(buffer2 + i + 1, buffer, count))
            start = i + 1;

    }

}

