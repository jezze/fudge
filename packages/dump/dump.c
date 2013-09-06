#include <fudge.h>

void main()
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count, roff, woff = 0;

    for (roff = 0; (count = call_read(CALL_DI, roff, FUDGE_BSIZE, buffer)); roff += count)
    {

        unsigned int i;

        for (i = 0; i < count; i++)
        {

            unsigned char num[32];

            woff += call_write(CALL_DO, woff, memory_write_paddednumber(num, 32, buffer[i], 16, 2, 0), num);
            woff += call_write(CALL_DO, woff, 2, "  ");

        }

    }

}

