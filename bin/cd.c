#include <fudge.h>

void get_path(char *buffer, char *arg)
{

    int cwd = file_open("/cwd");
    unsigned int count = file_read(cwd, 256, buffer);

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

    if (argc == 1)
    {

        get_path(path, 0);
        file_write_string(FILE_STDOUT, path);
        file_write_byte(FILE_STDOUT, '\n');

        return;

    }

    get_path(path, argv[1]);

    int new = file_open(path);

    if (new == -1)
    {

        file_write_string(FILE_STDOUT, "Directory does not exist.\n");
        file_close(new);

        return;

    }

    file_close(new);

    int cwd = file_open("/cwd");
    file_write_string(cwd, path);
    file_close(cwd);

    return;

}

