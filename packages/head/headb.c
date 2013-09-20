#include <fudge.h>

void main()
{

    char num[32];
    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count, roff, woff = 0;
    unsigned int total;

    memory_clear(num, 32);

    count = call_read(CALL_I1, 0, 32, num);

    if (count == 32)
        return;

    total = string_number(num, 10);

    for (roff = 0; (count = call_read(CALL_I0, roff, FUDGE_BSIZE, buffer)); roff += count)
    {

        woff += call_write(CALL_O0, woff, (total < woff + count) ? total : woff + count, buffer);

        if (woff >= total)
            break;

    }

}

