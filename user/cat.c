#include <fudge.h>

char buffer[0x4000];

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

    unsigned int count = file_read(fd, 0, 0x4000, buffer);

    file_close(fd);

    file_write(FILE_STDOUT, 0, count, buffer);

}

