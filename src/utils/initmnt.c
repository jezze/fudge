#include <fudge.h>
#include <abi.h>

void init(void)
{

}

void main(void)
{

    file_walk2(FILE_L0, "/system");
    call_auth(FILE_L1, 2000, 2000);
    call_mount(FILE_L0, FILE_L1);

}

