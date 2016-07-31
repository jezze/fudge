#include <abi.h>
#include <fudge.h>

void main(void)
{

    if (!call_walk(CALL_L0, CALL_PR, 7, "system/"))
        return;

    call_auth(CALL_L1, 2000);
    call_mount(CALL_L0, CALL_L1);

}

