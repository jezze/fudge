#include <fudge.h>

void main(int argc, char *argv[])
{

    file_write_format(FILE_STDOUT, 0, "System is rebooting...");
    call_reboot();

}

