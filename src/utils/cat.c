#include <abi.h>
#include <fudge.h>

void main(void)
{

    if (!file_walk(CALL_CP, "/bin/echo"))
        return;

    if (!file_duplicate(CALL_CI, CALL_PI + 0))
        return;

    call_spawn();

    if (!file_duplicate(CALL_CI, CALL_PI + 1))
        return;

    call_spawn();

}

