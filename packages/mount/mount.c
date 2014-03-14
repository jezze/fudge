#include <fudge.h>

void main()
{

    if (call_walk(CALL_L0, CALL_DR, 7, "system/"))
    {

        if (call_walk(CALL_L1, CALL_DR, 18, "boot/mod/system.ko"))
        {

            call_mount(2, CALL_L0, CALL_L1);
            call_close(CALL_L1);

        }

        call_close(CALL_L0);

    }

    if (call_walk(CALL_L0, CALL_DR, 5, "temp/"))
    {

        if (call_walk(CALL_L1, CALL_DR, 16, "boot/mod/temp.ko"))
        {

            call_mount(3, CALL_L0, CALL_L1);
            call_close(CALL_L1);

        }

        call_close(CALL_L0);

    }

}

