#include <call.h>
#include <file.h>

void main(int argc, char *argv[])
{

    int fd = file_open("/dev/rtc");

    unsigned char buffer;

    file_write_string2(FILE_STDOUT, "20");
    file_read2(fd, 0, 1, &buffer);
    file_write_bcd2(FILE_STDOUT, buffer);

    file_write_string2(FILE_STDOUT, "-");

    file_read2(fd, 1, 1, &buffer);
    file_write_bcd2(FILE_STDOUT, buffer);

    file_write_string2(FILE_STDOUT, "-");

    file_read2(fd, 2, 1, &buffer);
    file_write_bcd2(FILE_STDOUT, buffer);

    file_write_string2(FILE_STDOUT, " ");

    file_read2(fd, 3, 1, &buffer);
    file_write_bcd2(FILE_STDOUT, buffer);

    file_write_string2(FILE_STDOUT, ":");

    file_read2(fd, 4, 1, &buffer);
    file_write_bcd2(FILE_STDOUT, buffer);

    file_write_string2(FILE_STDOUT, ":");

    file_read2(fd, 5, 1, &buffer);
    file_write_bcd2(FILE_STDOUT, buffer);

    file_read2(fd, 3, 1, &buffer);

    if (buffer >> 4)
        file_write_string2(FILE_STDOUT, "PM\n");
    else
        file_write_string2(FILE_STDOUT, "AM\n");

    file_close(fd);

}

