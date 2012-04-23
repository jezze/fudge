#include <fudge.h>

void main(int argc, char *argv[])
{

    char buffer[256];

    unsigned int fd = file_open("/module/tty/cwd");

    if (argc == 1)
    {

        unsigned int count = file_read(fd, 0, 256, buffer);
        file_write(FILE_STDOUT, 0, count, buffer);
        file_write(FILE_STDOUT, 0, 1, "\n");

    }

    if (argc == 2)
    {

        file_write(fd, 0, string_length(argv[1]), argv[1]);


    }

    file_close(fd);

}

