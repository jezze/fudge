#include <abi.h>
#include <fudge.h>

void main()
{

    if (!call_walk(CALL_L0, CALL_PR, 12, "system/reset"))
        return;

    call_open(CALL_PO);
    call_write(CALL_PO, 0, 23, 1, "System is rebooting...\n");
    call_close(CALL_PO);
    call_open(CALL_L0);
    call_write(CALL_L0, 0, 1, 1, "1");
    call_close(CALL_L0);

}

