#include <fudge.h>

void main(int argc, char *argv[])
{

    file_write(FILE_STDOUT, 0, 23, "System is rebooting...\n");

    call_reboot();

}

