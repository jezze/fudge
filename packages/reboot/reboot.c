#include <arch.h>
#include <fudge.h>

void main()
{

    if (!call_open(CALL_D0, CALL_DR, 12, "system/reset"))
        return;

    call_write(CALL_DO, 0, 23, "System is rebooting...\n");
    call_write(CALL_D0, 0, 1, "1");
    call_close(CALL_D0);

}

