#include <fudge.h>

void main(int argc, char *argv[])
{

    char buffer[32];

    unsigned int fd = file_open("/module/pit/jiffies");

    if (!fd)
    {

        file_write_format(FILE_STDOUT, 0, "Could not read pit/jiffies\n");

        return;

    }

    file_read(fd, 0, 32, buffer);
    file_close(fd);

    file_write_format(FILE_STDOUT, 0, "Timer: %s\n", buffer);

}

