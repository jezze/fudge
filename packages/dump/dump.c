#include <arch.h>
#include <fudge.h>

void main()
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;
    unsigned int offset;
    unsigned int last = 0;

    for (offset = 0; (count = call_read(CALL_DI, offset, FUDGE_BSIZE, buffer)); offset += count)
    {

        unsigned char num[32];
        unsigned int i;

        for (i = 0; i < count; i++)
        {

            last += call_write(CALL_DO, last, memory_write_paddednumber(num, 32, buffer[i], 16, 2, 0), num);
            last += call_write(CALL_DO, last, 2, "  ");

        }

    }

}

