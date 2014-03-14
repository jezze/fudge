#include <fudge.h>

void main()
{

    if (!call_walk(CALL_DW, CALL_DR, 5, "home/"))
        return;

    call_open(CALL_DW);

    if (call_walk(CALL_I1, CALL_DR, 17, "config/init.slang"))
    {

        call_open(CALL_I1);

        if (call_walk(CALL_L0, CALL_DR, 9, "bin/slang"))
        {

            call_open(CALL_L0);
            call_spawn(CALL_L0);
            call_close(CALL_L0);

        }

        call_close(CALL_I1);

    }

    call_close(CALL_DW);

}

