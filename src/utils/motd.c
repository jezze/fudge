#include <abi.h>
#include <fudge.h>
#include <lib/file.h>

void main(void)
{

    call_open(CALL_PO);
    file_writeall(CALL_PO, "FUDGE OPERATING SYSTEM\n\n", 24);
    call_close(CALL_PO);

    if (!call_walk(CALL_CP, CALL_PR, 8, "bin/echo"))
        return;

    if (!call_walk(CALL_CI, CALL_PR, 14, "share/motd.txt"))
        return;

    call_spawn();

}

