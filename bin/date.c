#include <fudge.h>

void write_bcd(unsigned int fd, unsigned char num)
{

    file_write_format(fd, "%d", num >> 4);
    file_write_format(fd, "%d", num & 0x0F);

}

void main(int argc, char *argv[])
{

    int fd = file_open("/rtc");

    unsigned char buffer;

    file_write_format(FILE_STDOUT, "20");
    file_read(fd, 1, &buffer);
    write_bcd(FILE_STDOUT, buffer);

    file_write_format(FILE_STDOUT, "-");

    file_read(fd, 1, &buffer);
    write_bcd(FILE_STDOUT, buffer);

    file_write_format(FILE_STDOUT, "-");

    file_read(fd, 1, &buffer);
    write_bcd(FILE_STDOUT, buffer);

    file_write_format(FILE_STDOUT, " ");

    file_read(fd, 1, &buffer);
    write_bcd(FILE_STDOUT, buffer);

    file_write_format(FILE_STDOUT, ":");

    file_read(fd, 1, &buffer);
    write_bcd(FILE_STDOUT, buffer);

    file_write_format(FILE_STDOUT, ":");

    file_read(fd, 1, &buffer);
    write_bcd(FILE_STDOUT, buffer);

    file_read(fd, 1, &buffer);

    if (buffer >> 4)
        file_write_format(FILE_STDOUT, "PM\n");
    else
        file_write_format(FILE_STDOUT, "AM\n");

    file_close(fd);

}

