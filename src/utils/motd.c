#include <abi.h>
#include <fudge.h>

void main(void)
{

    if (!call_walk(CALL_CP, CALL_PR, 8, "bin/echo"))
        return;

    if (!call_walk(CALL_CI, CALL_PR, 14, "share/motd.txt"))
        return;

    call_spawn();

}

