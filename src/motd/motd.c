#include <abi.h>
#include <fudge.h>

void main()
{

    unsigned int woff = 0;

    call_open(CALL_PO);

    woff += call_write(CALL_PO, woff, 23, 1, "FUDGE OPERATING SYSTEM\n");
    woff += call_write(CALL_PO, woff, 12, 1, "Build date: ");
    woff += call_write(CALL_PO, woff, 11, 1, __DATE__);
    woff += call_write(CALL_PO, woff, 1, 1, " ");
    woff += call_write(CALL_PO, woff, 8, 1, __TIME__);
    woff += call_write(CALL_PO, woff, 2, 1, "\n\n");

    call_close(CALL_PO);

    if (!call_walk(CALL_CP, CALL_PR, 8, "bin/echo"))
        return;

    if (!call_walk(CALL_C0, CALL_PR, 14, "share/motd.txt"))
        return;

    call_spawn();

}

