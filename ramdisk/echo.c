#include <vfs.h>
#include <stdout.h>

void main(int argc, char *argv[])
{

    if (argc != 2)
        return;

    stdout_write(argv[1]);

}

