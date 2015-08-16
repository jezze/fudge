#include <abi.h>
#include <fudge.h>

void main(void)
{

    call_open(CALL_PO);
    call_write(CALL_PO, 0, 24, 1, "FUDGE OPERATING SYSTEM\n\n");
    call_close(CALL_PO);

    if (!call_walk(CALL_CP, CALL_PR, 8, "bin/echo"))
        return;

    if (!call_walk(CALL_C0, CALL_PR, 14, "share/motd.txt"))
        return;

    call_spawn();

}

