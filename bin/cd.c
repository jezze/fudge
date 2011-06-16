#include <call.h>
#include <file.h>
#include <string.h>

void main(int argc, char *argv[])
{

    int cwd = call_open2("/dev/cwd");

    char buffer[256];

    unsigned int count = call_read(cwd, buffer, 10);

    if (argc == 1)
    {

        file_write_string2(FILE_STDOUT, buffer);
        file_write_string2(FILE_STDOUT, "\n");

        call_close(cwd);

        return;

    }

    if (argv[1][string_length(argv[1]) - 1] != '/')
        string_concat(argv[1], "/");

    if (argv[1][0] == '/')
        string_copy(buffer, argv[1]);
    else
        string_concat(buffer, argv[1]);

    int new = call_open2(buffer);

    if (new == -1)
    {

        file_write_string2(FILE_STDOUT, "Directory does not exist.\n");

        call_close(new);
        call_close(cwd);

        return;

    }

    file_write_string2(cwd, buffer);

    call_close(new);
    call_close(cwd);

    return;

}

