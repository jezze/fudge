#include <abi.h>
#include <fudge.h>

void main(void)
{

    if (!file_walk(CALL_CW, "/home/"))
        return;

    if (!file_walk(CALL_CP, "/bin/slang"))
        return;

    if (!file_walk(CALL_CI, "/config/init.slang"))
        return;

    call_spawn();

}

