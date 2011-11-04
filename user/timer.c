#include <fudge.h>

void main(int argc, char *argv[])
{

    char buffer[64];

    int fd = file_open("/jiffies");
    file_read(fd, 64, buffer);
    file_write_format(FILE_STDOUT, "Timer: %s\n", buffer);
    file_close(fd);

    call_exit();

}

