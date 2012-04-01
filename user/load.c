#include <fudge.h>

void main(int argc, char *argv[])
{

    if (argc != 2)
    {

        file_write_format(FILE_STDOUT, 0, "You need to supply filename.\n");

        return;

    }

    unsigned int fd = file_open(argv[1]);

    if (!fd)
    {

        file_write_format(FILE_STDOUT, 0, "%s: File does not exist.\n", argv[1]);

        return;

    }

    file_close(fd);

    call_load(argv[1]);

}

