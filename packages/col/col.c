#include <fudge.h>

void main()
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;
    unsigned int offset;
    unsigned int start;
    unsigned int last = 0;

    for (offset = 0; (count = call_read(CALL_DI, offset, FUDGE_BSIZE, buffer)); offset += start)
    {

        unsigned int i;

        start = 0;

        for (i = 0; i < count; i++)
        {

            unsigned int l;

            if (buffer[i] != '\n')
                continue;

            l = i - start;

            if (l > 18)
                l = 18;

            last += call_write(CALL_DO, last, l, buffer + start);
            last += call_write(CALL_DO, last, 20 - l, "                    ");

            start = i + 1;

        }

        if (!start)
            return;

    }

    call_write(CALL_DO, last, 1, "\n");

}

