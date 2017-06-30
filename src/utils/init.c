#include <abi.h>
#include <fudge.h>

void main(void)
{

    if (!file_walk(FILE_CW, "/home/"))
        return;

    if (!file_walk(FILE_CP, "/bin/slang"))
        return;

    if (!file_walk(FILE_CI, "/config/init.slang"))
        return;

    call_spawn();

}

