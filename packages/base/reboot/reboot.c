#include <fudge.h>

void main()
{

    call_write(FUDGE_OUT, 0, 23, "System is rebooting...\n");

    if (!call_open(FUDGE_IN, FUDGE_ROOT, 15, "system/ps2reset"))
        return;

    call_write(FUDGE_IN, 0, 1, "1");

}

