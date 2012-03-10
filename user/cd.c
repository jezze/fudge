#include <fudge.h>

void main(int argc, char *argv[])
{

    char buffer[256];

    unsigned int fd = file_open("/module/tty/cwd");

    if (argc == 1)
    {

        file_read(fd, 256, buffer);
        file_write_format(FILE_STDOUT, "%s\n", buffer);

    }

    if (argc == 2)
    {

        file_write(fd, string_length(argv[1]), argv[1]);


    }

    file_close(fd);

}

