#include <fudge.h>

void main(int argc, char *argv[])
{

    char buffer[32];

    unsigned int fd = call_open("/module/pit/jiffies");

    if (!fd)
        return;

    unsigned int count = call_read(fd, 0, 32, buffer);
    call_close(fd);
    call_write(FILE_STDOUT, 0, count, buffer);

}

