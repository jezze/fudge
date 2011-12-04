#include <fudge.h>

void main(int argc, char *argv[])
{

    char buffer[32];

    int fd = file_open("pit", "jiffies");
    file_read(fd, 32, buffer);
    file_write_format(FILE_STDOUT, "Timer: %s\n", buffer);
    file_close(fd);

    call_exit();

}

