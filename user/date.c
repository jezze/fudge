#include <fudge.h>

void main(int argc, char *argv[])
{

    char buffer[64];

    unsigned int fd = call_open("/module/rtc/timestamp");

    if (!fd)
        return;

    unsigned int count = call_read(fd, 0, 64, buffer);
    call_close(fd);

    call_write(FILE_STDOUT, 0, count, buffer);

}

