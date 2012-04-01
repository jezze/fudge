#include <fudge.h>

void main(int argc, char *argv[])
{

    char buffer[64];

    unsigned int fd = file_open("/module/rtc/timestamp");

    if (!fd)
    {

        file_write_format(FILE_STDOUT, 0, "Could not read rtc/timestamp\n");

        return;

    }

    file_read(fd, 0, 64, buffer);
    file_close(fd);

    file_write_format(FILE_STDOUT, 0, "Date: %s\n", buffer);

}

