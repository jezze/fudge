#include <vfs.h>
#include <call.h>

void main(int argc, char *argv[])
{

    call_puts("System halt.");
    call_halt();

}

