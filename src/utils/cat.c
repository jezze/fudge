#include <abi.h>
#include <fudge.h>

void main(void)
{

    if (!file_walk(FILE_CP, "/bin/echo"))
        return;

    if (!file_duplicate(FILE_CI, FILE_PI + 0))
        return;

    call_spawn();

    if (!file_duplicate(FILE_CI, FILE_PI + 1))
        return;

    call_spawn();

}

