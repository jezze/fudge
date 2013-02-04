#include <fudge.h>

void main()
{

    call_write(FUDGE_OUT, 0, 23, "System is rebooting...\n");
    call_open(3, FUDGE_ROOT, 16, "system/ps2_reset");
    call_write(3, 0, 1, "1");

}

