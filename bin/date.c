#include <fudge.h>

void main(int argc, char *argv[])
{

    int fd = file_open("/rtc");

    unsigned char buffer;

    file_write_format(FILE_STDOUT, "20");
    file_read(fd, 1, &buffer);
    file_write_bcd(FILE_STDOUT, buffer);

    file_write_format(FILE_STDOUT, "-");

    file_read(fd, 1, &buffer);
    file_write_bcd(FILE_STDOUT, buffer);

    file_write_format(FILE_STDOUT, "-");

    file_read(fd, 1, &buffer);
    file_write_bcd(FILE_STDOUT, buffer);

    file_write_format(FILE_STDOUT, " ");

    file_read(fd, 1, &buffer);
    file_write_bcd(FILE_STDOUT, buffer);

    file_write_format(FILE_STDOUT, ":");

    file_read(fd, 1, &buffer);
    file_write_bcd(FILE_STDOUT, buffer);

    file_write_format(FILE_STDOUT, ":");

    file_read(fd, 1, &buffer);
    file_write_bcd(FILE_STDOUT, buffer);

    file_read(fd, 1, &buffer);

    if (buffer >> 4)
        file_write_format(FILE_STDOUT, "PM\n");
    else
        file_write_format(FILE_STDOUT, "AM\n");

    file_close(fd);

}

