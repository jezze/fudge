#include <fudge.h>

void main()
{

    if (!call_walk(CALL_DW, CALL_DR, 5, "home/"))
        return;

    if (call_walk(CALL_I1, CALL_DR, 17, "config/init.slang"))
    {

        if (call_walk(CALL_L0, CALL_DR, 9, "bin/slang"))
        {

            call_spawn(CALL_L0);
            call_close(CALL_L0);

        }

        call_close(CALL_I1);

    }

    call_close(CALL_DW);

}

