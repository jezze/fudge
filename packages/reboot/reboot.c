#include <fudge.h>

void main()
{

    if (!call_walk(CALL_L0, CALL_DR, 12, "system/reset"))
        return;

    call_open(CALL_O0, CALL_OPEN_WRITE);
    call_write(CALL_O0, 0, 23, "System is rebooting...\n");
    call_close(CALL_O0);
    call_open(CALL_L0, CALL_OPEN_WRITE);
    call_write(CALL_L0, 0, 1, "1");
    call_close(CALL_L0);

}

