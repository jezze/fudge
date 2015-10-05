#include <abi.h>
#include <fudge.h>

void main(void)
{

    if (!call_walk(CALL_CP, CALL_PR, 8, "bin/echo"))
        return;

    if (!call_walk(CALL_CI, CALL_PI + 0, 0, 0))
        return;

    call_spawn();

    if (!call_walk(CALL_CI, CALL_PI + 1, 0, 0))
        return;

    call_spawn();

}

