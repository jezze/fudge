#include <abi.h>
#include <fudge.h>
#include <lib/file.h>

void main(void)
{

    if (!call_walk(CALL_L0, CALL_PR, 7, "system/"))
        return;

    call_auth(CALL_L1, 4, 2000);
    call_mount(4, CALL_L0, CALL_L1);

}

