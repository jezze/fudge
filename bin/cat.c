#include <call.h>
#include <file.h>
#include <string.h>

void get_path(char *buffer, char *arg)
{

    int cwd = file_open("/dev/cwd");
    unsigned int count = file_read(cwd, 0, 256, buffer);

    if (arg)
    {

        if (arg[0] == '/')
            string_copy(buffer, arg);
        else
            string_concat(buffer, arg);

    }

    if (buffer[string_length(buffer) - 1] != '/')
        string_concat(buffer, "/");

    file_close(cwd);

}

void main(int argc, char *argv[])
{

    char path[256];
    char buffer[512];

    if (argc != 2)
    {

        file_write_string(FILE_STDOUT, "You need to supply filename.\n");

        return;

    }

    get_path(path, argv[1]);

    int fd = file_open(path);

    if (fd == -1)
    {

        file_write_string(FILE_STDOUT, "File does not exist.\n");

        return;

    }

    unsigned int count = file_read(fd, 0, 512, buffer);

    file_write(FILE_STDOUT, 0, count, buffer);
    file_close(fd);

}

