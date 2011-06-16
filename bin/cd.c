#include <call.h>
#include <file.h>
#include <string.h>

void main(int argc, char *argv[])
{

    int cwd = file_open("/dev/cwd");

    char path[256];

    unsigned int count = file_read2(cwd, 0, 256, path);

    if (argc == 1)
    {

        file_write_string2(FILE_STDOUT, path);
        file_write_string2(FILE_STDOUT, "\n");

        file_close(cwd);

        return;

    }

    if (argv[1][string_length(argv[1]) - 1] != '/')
        string_concat(argv[1], "/");

    if (argv[1][0] == '/')
        string_copy(path, argv[1]);
    else
        string_concat(path, argv[1]);

    int new = file_open(path);

    if (new == -1)
    {

        file_write_string2(FILE_STDOUT, "Directory does not exist.\n");

        file_close(new);
        file_close(cwd);

        return;

    }

    file_write_string2(cwd, path);

    file_close(new);
    file_close(cwd);

    return;

}

