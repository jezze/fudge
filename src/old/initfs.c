#include <abi.h>
#include <fudge.h>

void main(void)
{

    if (!file_walk(FILE_L0, "/system"))
        return;

    call_auth(FILE_L1, 2000, 2000);
    call_mount(FILE_L0, FILE_L1);

/*
    if (!file_walk(FILE_L0, "/proc"))
        return;

    call_auth(FILE_L1, 3001, 3001);
    call_mount(FILE_L0, FILE_L1);
*/

}

