#include <fudge.h>

void get_path(char *buffer, char *arg)
{

    int cwd = file_open("/cwd");
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

    call_unload(path);

    call_exit();

}

