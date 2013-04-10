#include <fudge.h>

void main()
{

    if (!call_open(FUDGE_DI, FUDGE_DR, 12, "system/reset"))
        return;

    call_write(FUDGE_DO, 0, 23, "System is rebooting...\n");
    call_write(FUDGE_DI, 0, 1, "1");

}

