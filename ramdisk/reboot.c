#include <call.h>
#include <vfs.h>

void main(int argc, char *argv[])
{

    call_puts("System is rebooting...");
    call_reboot();

}

