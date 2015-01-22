#include <abi.h>
#include <fudge.h>

void main()
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count, roff, woff = 0;

    if (!call_walk(CALL_L0, CALL_DR, 14, "share/help.txt"))
        return;

    call_open(CALL_O0);
    call_open(CALL_L0);

    for (roff = 0; (count = call_read(CALL_L0, roff, FUDGE_BSIZE, buffer)); roff += count)
        woff += call_write(CALL_O0, woff, count, buffer);

    call_close(CALL_L0);
    call_close(CALL_O0);

}

