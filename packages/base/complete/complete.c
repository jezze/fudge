#include <fudge.h>

void main()
{

    char buffermatch[FUDGE_BSIZE];
    char buffer[FUDGE_BSIZE];
    unsigned int countmatch = call_read(FUDGE_IN, 0, FUDGE_BSIZE, buffermatch);
    unsigned int count;
    unsigned int offset;
    unsigned int start;
    unsigned int o = 0;

    for (offset = 0; (count = call_read(FUDGE_CWD, offset, FUDGE_BSIZE, buffer)); offset += start)
    {

        unsigned int i;

        start = 0;

        for (i = 0; i < count; i++)
        {

            if (buffer[i] != '\n')
                continue;

            if (memory_match(buffer + start, buffermatch, countmatch))
                o += call_write(FUDGE_OUT, o, i - start + 1, buffer + start);

            start = i + 1;

        }

        if (!start)
            return;

    }

}

