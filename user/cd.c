#include <fudge.h>

void main(int argc, char *argv[])
{

    char buffer[256];

    unsigned int fd = file_open("tty/cwd");

    if (argc == 1)
    {

        file_read(fd, 256, buffer);

        file_write_format(FILE_STDOUT, buffer);
        file_write_byte(FILE_STDOUT, '\n');

    }

    if (argc == 2)
    {

        file_write(fd, string_length(argv[1]), argv[1]);


    }

    file_close(fd);

}

