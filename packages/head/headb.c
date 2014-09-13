#include <fudge.h>

void main()
{

    char num[32];
    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count, roff, woff = 0;
    unsigned int total;

    call_open(CALL_I1);

    count = call_read(CALL_I1, 0, 32, num);

    call_close(CALL_I1);

    if (count == 32)
        return;

    total = ascii_rvalue(num, count, 10);

    call_open(CALL_O0);
    call_open(CALL_I0);

    for (roff = 0; (count = call_read(CALL_I0, roff, FUDGE_BSIZE, buffer)); roff += count)
    {

        woff += call_write(CALL_O0, woff, (total < woff + count) ? total : woff + count, buffer);

        if (woff >= total)
            break;

    }

    call_close(CALL_I0);
    call_close(CALL_O0);

}

