#include <fudge.h>

void main(int argc, char *argv[])
{

    if (argc != 2)
    {

        file_write(FILE_STDOUT, 0, 29, "You need to supply filename.\n");

        return;

    }

    unsigned int fd = file_open(argv[1]);

    if (!fd)
    {

        file_write(FILE_STDOUT, 0, string_length(argv[1]), argv[1]);
        file_write(FILE_STDOUT, 0, 23, ": File does not exist.\n");


        return;

    }

    file_close(fd);

    call_load(argv[1]);

}

