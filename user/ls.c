#include <fudge.h>

char buffer[0x1000];

void main(int argc, char *argv[])
{

    unsigned int fd = file_open("/module/tty/pwd");

    if (!fd)
    {

        file_write_format(FILE_STDOUT, "%s: File does not exist.\n", argv[1]);

        return;

    }

    unsigned int count = file_read(fd, 0x1000, buffer);

    file_close(fd);

    file_write(FILE_STDOUT, count, buffer);

}

