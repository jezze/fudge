#include <fudge.h>

void main(int argc, char *argv[])
{

    char buffer[0x1000];

    unsigned int fd = file_open("/module/tty/pwd");
    unsigned int count = file_read(fd, 0, 0x1000, buffer);
    file_close(fd);

    file_write(FILE_STDOUT, 0, count, buffer);

}

