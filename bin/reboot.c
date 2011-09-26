#include <fudge.h>

void main(int argc, char *argv[])
{

    file_write_string(FILE_STDOUT, "System is rebooting...");
    call_reboot();

}

