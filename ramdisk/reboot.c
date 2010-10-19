#include <call.h>
#include <session.h>
#include <file.h>

void main(int argc, char *argv[])
{

    file_write(session_get_out(), "System is rebooting...");
    call_reboot();

}

