#include <call.h>
#include <file.h>

void main(int argc, char *argv[])
{

    if (argc != 2)
    {

        file_write_string2(FILE_STDOUT, "You need to supply filename.\n");

        return;

    }

    int fd = file_open(argv[1]);

    if (fd == -1)
    {

        file_write_string2(FILE_STDOUT, "File does not exist.\n");

        return;

    }

    char buffer[500];

    unsigned int count = file_read2(fd, 0, 500, buffer);

    file_write2(FILE_STDOUT, 0, count, buffer);

    file_close(fd);

}

