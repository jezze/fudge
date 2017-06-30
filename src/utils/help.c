#include <abi.h>
#include <fudge.h>

void main(void)
{

    if (!file_walk(FILE_CP, "/bin/echo"))
        return;

    if (!file_walk(FILE_CI, "/data/help.txt"))
        return;

    call_spawn();

}

