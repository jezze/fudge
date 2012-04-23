#include <fudge.h>

void main(int argc, char *argv[])
{

    if (argc != 2)
    {

        file_write(FILE_STDOUT, 0, 29, "You need to supply filename.\n");

        return;

    }

    call_unload(argv[1]);

}

