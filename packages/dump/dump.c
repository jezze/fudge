#include <fudge.h>

void main()
{

    char buffer[FUDGE_BSIZE];
    unsigned int count;
    unsigned int i;

    count = call_read(CALL_DI, 0, FUDGE_BSIZE, buffer);

    for (i = 0; i < count; i++)
    {

        char num[32];
        unsigned int c = memory_write_number(num, 32, buffer[i], 16, 0);

        call_write(CALL_DO, 0, c, num);
        call_write(CALL_DO, 0, 1, ":");

    }

}

