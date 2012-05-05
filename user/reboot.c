#include <fudge.h>

void main(int argc, char *argv[])
{

    call_write(FILE_STDOUT, 0, 23, "System is rebooting...\n");
    call_reboot();

}

