#include <fudge.h>

void main()
{

    if (!call_open(CALL_DI, CALL_DR, 12, "system/reset"))
        return;

    call_write(CALL_DO, 0, 23, "System is rebooting...\n");
    call_write(CALL_DI, 0, 1, "1");

}

