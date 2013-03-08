#include <fudge.h>

void main()
{

    char buffer[FUDGE_BSIZE];
    char buffer2[FUDGE_BSIZE];
    unsigned int count = call_read(FUDGE_IN, 0, FUDGE_BSIZE, buffer);
    unsigned int count2 = call_read(FUDGE_CWD, 0, FUDGE_BSIZE, buffer2);
    unsigned int start = 0;
    unsigned int i;

    for (i = 0; i < count2; i++)
    {

        if (buffer2[i] != '\n')
            continue;

        if (memory_match(buffer2 + start, buffer, count))
            call_write(FUDGE_OUT, 0, i - start + 1, buffer2 + start);

        start = i + 1;

    }

}

