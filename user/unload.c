#include <fudge.h>

void main(int argc, char *argv[])
{

    if (argc != 2)
    {

        file_write_format(FILE_STDOUT, "You need to supply filename.\n");

        call_exit();

    }

    call_unload(argv[1]);

}

