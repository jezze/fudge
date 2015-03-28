#include <abi.h>
#include <fudge.h>

void main()
{

    if (!call_walk(CALL_CP, CALL_PR, 8, "bin/echo"))
        return;

    if (!call_walk(CALL_C0, CALL_PR, 14, "share/help.txt"))
        return;

    call_spawn();

}

