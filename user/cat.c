#include <fudge.h>

char buffer[0x4000];

void get_path(char *buffer, char *arg)
{

    int cwd = file_open("dev", "/cwd");
    unsigned int count = file_read(cwd, 256, buffer);

    if (arg)
    {

        if (arg[0] == '/')
            string_write(buffer, arg);
        else
            string_write_concat(buffer, arg);

    }

    file_close(cwd);

}

void main(int argc, char *argv[])
{

    char path[256];

    if (argc != 2)
    {

        file_write_format(FILE_STDOUT, "You need to supply filename.\n");

        call_exit();

    }

    get_path(path, argv[1]);

    int fd = file_open("initrd", path);

    if (!fd)
    {

        file_write_format(FILE_STDOUT, "%s: File does not exist.\n", path);

        call_exit();

    }

    unsigned int count = file_read(fd, 0x4000, buffer);

    file_write(FILE_STDOUT, count, buffer);
    file_close(fd);

    call_exit();

}

