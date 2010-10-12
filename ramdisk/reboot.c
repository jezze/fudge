#include <call.h>
#include <write.h>

void main(int argc, char *argv[])
{

    write_string("System is rebooting...");
    call_reboot();

}

