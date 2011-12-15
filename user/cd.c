#include <fudge.h>

void get_path(char *buffer)
{

    int fd = file_open("tty", "cwd");
    file_read(fd, 256, buffer);
    file_close(fd);

}

void set_path(char *buffer)
{

    int fd = file_open("tty", "cwd");
    file_write(fd, string_length(buffer), buffer);
    file_close(fd);

}

void main(int argc, char *argv[])
{

    char path[256];

    get_path(path);

    if (argc == 1)
    {

        file_write_format(FILE_STDOUT, path);
        file_write_byte(FILE_STDOUT, '\n');

        return;

    }

    set_path(argv[1]);

}

