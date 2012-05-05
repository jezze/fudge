#include <fudge.h>

#define BUFFER_SIZE 0x100

void main(int argc, char *argv[])
{

    char buffer[BUFFER_SIZE];

    unsigned int fd = call_open("/module/rtc/timestamp");

    if (!fd)
        return;

    unsigned int count = call_read(fd, 0, BUFFER_SIZE, buffer);
    call_close(fd);
    call_write(FILE_STDOUT, 0, count, buffer);

}

