#include <call.h>
#include <file.h>

void main(int argc, char *argv[])
{

    if (argc != 2)
    {

        file_write_string2(FILE_STDOUT, "You need to supply filename.\n");

        return;

    }

    int fd = call_open2(argv[1]);

    if (fd == -1)
    {

        file_write_string2(FILE_STDOUT, "File does not exist.\n");

        return;

    }

    char buffer[2000];

    unsigned int count = call_read(fd, buffer, 1000);

    file_write2(FILE_STDOUT, 0, count, buffer);

    call_close(fd);

}

