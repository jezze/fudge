#include <fudge.h>

void main(int argc, char *argv[])
{

    int fd = file_open("/tty");

    char c = ' ';
    unsigned int i;

    for (i = 2000; i; i--)
        file_write_byte(fd, c);

    file_close(fd);

}

