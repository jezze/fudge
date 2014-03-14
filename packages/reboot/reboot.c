#include <fudge.h>

void main()
{

    if (call_walk(CALL_L0, CALL_DR, 12, "system/reset"))
    {

        call_write(CALL_O0, 0, 23, "System is rebooting...\n");
        call_write(CALL_L0, 0, 1, "1");
        call_close(CALL_L0);

    }

}

