#include <fudge.h>

char buffer[0x4000];

void get_path(char *buffer)
{

    int fd = file_open("/module/tty/cwd");
    file_read(fd, 0, 256, buffer);
    file_close(fd);

}

void main(int argc, char *argv[])
{

    char path[256];

    if (argc != 2)
    {

        file_write_format(FILE_STDOUT, 0, "You need to supply filename.\n");

        return;

    }

    get_path(path);

    int fd = file_open(argv[1]);

    if (!fd)
    {

        file_write_format(FILE_STDOUT, 0, "%s: File does not exist.\n", argv[1]);

        return;

    }

    for (;;)
    {

        unsigned int count = file_read(fd, 0, 0x4000, buffer);

        file_write(FILE_STDOUT, 0, count, buffer);

    }

    file_close(fd);

}

