#include <call.h>
#include <file.h>

void main(int argc, char *argv[])
{

    file_write("System is rebooting...");
    call_reboot();

}

