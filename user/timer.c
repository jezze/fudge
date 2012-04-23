#include <fudge.h>

void main(int argc, char *argv[])
{

    char buffer[32];

    unsigned int fd = file_open("/module/pit/jiffies");

    if (!fd)
        return;

    unsigned int count = file_read(fd, 0, 32, buffer);
    file_close(fd);

    file_write(FILE_STDOUT, 0, count, buffer);

}

