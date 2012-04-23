#include <fudge.h>

void main(int argc, char *argv[])
{

    char buffer[64];

    unsigned int fd = file_open("/module/rtc/timestamp");

    if (!fd)
        return;

    unsigned int count = file_read(fd, 0, 64, buffer);
    file_close(fd);

    file_write(FILE_STDOUT, 0, count, buffer);

}

