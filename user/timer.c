#include <fudge.h>

void main(int argc, char *argv[])
{

    char buffer[32];

    int fd = file_open("pit/jiffies");

    if (!fd)
    {

        file_write_format(FILE_STDOUT, "Could not read pit/jiffies\n");

        return;

    }

    file_read(fd, 32, buffer);
    file_write_format(FILE_STDOUT, "Timer: %s\n", buffer);
    file_close(fd);

}

