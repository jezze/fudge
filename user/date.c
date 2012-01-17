#include <fudge.h>

void main(int argc, char *argv[])
{

    char buffer[64];

    int fd = file_open("rtc/date");

    if (!fd)
    {

        file_write_format(FILE_STDOUT, "Could not read rtc/date\n");

        return;

    }

    file_read(fd, 64, buffer);
    file_write_format(FILE_STDOUT, "Date: %s\n", buffer);
    file_close(fd);

}

