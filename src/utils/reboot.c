#include <fudge.h>
#include <abi.h>

void init(void)
{

}

void main(void)
{

    if (!file_walk2(FILE_L0, "/system/reset"))
        return;

    file_open(FILE_L0);
    file_writeall(FILE_L0, "1", 1);
    file_close(FILE_L0);

}

