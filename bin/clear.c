#include <call.h>
#include <file.h>

void main(int argc, char *argv[])
{

    int fd = call_open2("/dev/tty");

    char c = ' ';
    unsigned int i;

    for (i = 2000; i; i--)
        file_write2(fd, i - 1, 1, &c);

    call_close(fd);

}

