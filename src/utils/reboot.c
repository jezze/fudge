#include <abi.h>
#include <fudge.h>
#include <lib/file.h>

void main(void)
{

    if (!call_walk(CALL_L0, CALL_PR, 12, "system/reset"))
        return;

    call_open(CALL_PO);
    file_writeall(CALL_PO, "System is rebooting...\n", 23);
    call_close(CALL_PO);
    call_open(CALL_L0);
    file_writeall(CALL_L0, "1", 1);
    call_close(CALL_L0);

}

