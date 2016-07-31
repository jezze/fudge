#include <abi.h>
#include <fudge.h>

void main(void)
{

    if (!file_walk(CALL_CP, "/bin/echo"))
        return;

    if (!file_walk(CALL_CI, "/share/motd.txt"))
        return;

    call_spawn();

}

