#include <abi.h>
#include <fudge.h>

void main(void)
{

    if (!file_walk(FILE_L0, "/system/reset"))
        return;

    file_open(FILE_PO);
    file_writeall(FILE_PO, "System is rebooting...\n", 23);
    file_close(FILE_PO);
    file_open(FILE_L0);
    file_writeall(FILE_L0, "1", 1);
    file_close(FILE_L0);

}

