#include <abi.h>
#include <fudge.h>

void main(void)
{

    char num[32];
    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count, roff, woff = 0;
    unsigned int total;

    call_open(CALL_P1);

    count = call_read(CALL_P1, 0, 1, 32, num);

    call_close(CALL_P1);

    if (count == 32)
        return;

    total = ascii_rvalue(num, count, 10);

    call_open(CALL_PO);
    call_open(CALL_P0);

    for (roff = 0; (count = call_read(CALL_P0, roff, 1, FUDGE_BSIZE, buffer)); roff += count)
    {

        woff += call_write(CALL_PO, woff, (total < woff + count) ? total : woff + count, 1, buffer);

        if (woff >= total)
            break;

    }

    call_close(CALL_P0);
    call_close(CALL_PO);

}

