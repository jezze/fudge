#include <abi.h>
#include <fudge.h>

void main(void)
{

    if (!file_walk(CALL_L0, "/system/"))
        return;

    call_auth(CALL_L1, 2000);
    call_mount(CALL_L0, CALL_L1);

}

