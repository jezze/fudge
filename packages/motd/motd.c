#include <abi.h>
#include <fudge.h>

void main()
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count, roff, woff = 0;

    call_open(CALL_PO);

    woff += call_write(CALL_PO, woff, 23, "FUDGE OPERATING SYSTEM\n");
    woff += call_write(CALL_PO, woff, 12, "Build date: ");
    woff += call_write(CALL_PO, woff, 11, __DATE__);
    woff += call_write(CALL_PO, woff, 1, " ");
    woff += call_write(CALL_PO, woff, 8, __TIME__);
    woff += call_write(CALL_PO, woff, 2, "\n\n");

    call_close(CALL_PO);

    if (!call_walk(CALL_L0, CALL_PR, 14, "share/motd.txt"))
        return;

    call_open(CALL_PO);
    call_open(CALL_L0);

    for (roff = 0; (count = call_read(CALL_L0, roff, FUDGE_BSIZE, buffer)); roff += count)
        woff += call_write(CALL_PO, woff, count, buffer);

    call_close(CALL_L0);
    call_close(CALL_PO);

}

