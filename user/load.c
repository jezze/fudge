#include <fudge.h>

void main(int argc, char *argv[])
{

    if (argc != 2)
    {

        call_write(FILE_STDOUT, 0, 29, "You need to supply filename.\n");

        return;

    }

    unsigned int fd = call_open(argv[1]);

    if (!fd)
    {

        call_write(FILE_STDOUT, 0, string_length(argv[1]), argv[1]);
        call_write(FILE_STDOUT, 0, 23, ": File does not exist.\n");

        return;

    }

    call_close(fd);
    call_load(argv[1]);

}

