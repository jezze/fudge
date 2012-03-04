#include <fudge.h>

void main(int argc, char *argv[])
{

    char buffer[32];

    unsigned int fd = file_open("/module/pit/jiffies");

    if (!fd)
    {

        file_write_format(FILE_STDOUT, "Could not read pit/jiffies\n");

        return;

    }

    file_read(fd, 32, buffer);
    file_close(fd);

    file_write_format(FILE_STDOUT, "Timer: %s\n", buffer);

}

