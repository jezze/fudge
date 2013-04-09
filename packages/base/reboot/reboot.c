#include <fudge.h>

void main()
{

    if (!call_open(FUDGE_IN, FUDGE_ROOT, 12, "system/reset"))
        return;

    call_write(FUDGE_OUT, 0, 23, "System is rebooting...\n");
    call_write(FUDGE_IN, 0, 1, "1");

}

