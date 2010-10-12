#include <call.h>
#include <stdout.h>

void main(int argc, char *argv[])
{

    stdout_write("System is rebooting...");
    call_reboot();

}

