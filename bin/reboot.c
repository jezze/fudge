#include <call.h>
#include <file.h>

void main(int argc, char *argv[])
{

    file_write_string2(FILE_STDOUT, "System is rebooting...");
    call_reboot();

}

