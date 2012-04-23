#include <fudge.h>

char buffer[0x1000];

void main(int argc, char *argv[])
{

    unsigned int fd = file_open("/module/tty/pwd");

    if (!fd)
    {

        file_write(FILE_STDOUT, 0, string_length(argv[1]), argv[1]);
        file_write(FILE_STDOUT, 0, 23, ": File does not exist.\n");

        return;

    }

    unsigned int count = file_read(fd, 0, 0x1000, buffer);

    file_close(fd);

    file_write(FILE_STDOUT, 0, count, buffer);

}

