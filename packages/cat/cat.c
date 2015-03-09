#include <abi.h>
#include <fudge.h>

void main()
{

    if (!call_walk(CALL_CP, CALL_PR, 8, "bin/echo"))
        return;

    if (!call_walk(CALL_C0, CALL_P0, 0, 0))
        return;

    call_spawn();

    if (!call_walk(CALL_C0, CALL_P1, 0, 0))
        return;

    call_spawn();

}

