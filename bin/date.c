#include <call.h>
#include <file.h>

void main(int argc, char *argv[])
{

    struct file_node *node = call_open("/dev/rtc");

    unsigned char buffer;

    file_write_string2(FILE_STDOUT, "20");
    file_read(node, 0, 1, &buffer);
    file_write_bcd2(FILE_STDOUT, buffer);

    file_write_string2(FILE_STDOUT, "-");

    file_read(node, 1, 1, &buffer);
    file_write_bcd2(FILE_STDOUT, buffer);

    file_write_string2(FILE_STDOUT, "-");

    file_read(node, 2, 1, &buffer);
    file_write_bcd2(FILE_STDOUT, buffer);

    file_write_string2(FILE_STDOUT, " ");

    file_read(node, 3, 1, &buffer);
    file_write_bcd2(FILE_STDOUT, buffer);

    file_write_string2(FILE_STDOUT, ":");

    file_read(node, 4, 1, &buffer);
    file_write_bcd2(FILE_STDOUT, buffer);

    file_write_string2(FILE_STDOUT, ":");

    file_read(node, 5, 1, &buffer);
    file_write_bcd2(FILE_STDOUT, buffer);

    file_read(node, 3, 1, &buffer);

    if (buffer >> 4)
        file_write_string2(FILE_STDOUT, "PM\n");
    else
        file_write_string2(FILE_STDOUT, "AM\n");

}

