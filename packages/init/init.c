#include <abi.h>
#include <fudge.h>

void main()
{

    if (!call_walk(CALL_DW, CALL_DR, 5, "home/"))
        return;

    if (!call_walk(CALL_CPP, CALL_DR, 9, "bin/slang"))
        return;

    if (!call_walk(CALL_CI0, CALL_DR, 17, "config/init.slang"))
        return;

    call_spawn();

}

