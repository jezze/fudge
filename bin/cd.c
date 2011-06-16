#include <call.h>
#include <file.h>
#include <session.h>
#include <string.h>

void main(int argc, char *argv[])
{

    struct file_node *cwd = call_open("/dev/cwd");

    char buffer[256];

    unsigned int size = file_read(cwd, 0, 256, buffer);

    if (argc == 1)
    {

        file_write_string2(FILE_STDOUT, buffer);
        file_write_string2(FILE_STDOUT, "\n");

        return;

    }

    struct file_node *node = file_find(session_get_cwd(), argv[1]);

    if (!node)
    {

        file_write_string2(FILE_STDOUT, "Directory does not exist.\n");

        return;

    }

    if (!node->walk)
    {

        file_write_string2(FILE_STDOUT, "Not a directory.\n");

        return;

    }

    char c = '/';

    if (argv[1][0] == '/')
    {
        file_write(cwd, 0, string_length(argv[1]), argv[1]);

    }

    else
    {

        if (size > 1)
        {

            file_write(cwd, size, 1, &c);
            size++;

        }

        file_write(cwd, size, string_length(argv[1]), argv[1]);

    }

}

