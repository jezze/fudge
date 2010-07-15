#include <vfs.h>
#include <call.h>

void main(int argc, char *argv[])
{

    if (argc != 2)
        return;

    call_puts(argv[1]);

}

