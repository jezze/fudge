#include <fudge.h>

void main(int argc, char *argv[])
{

    char buffer[0x1000];

    unsigned int fd = call_open("/module/tty/pwd");
    unsigned int count = call_read(fd, 0, 0x1000, buffer);
    call_close(fd);
    call_write(FILE_STDOUT, 0, count, buffer);

}

