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

    if (argc == 1)
        get_path(path, 0);
    else
        get_path(path, argv[1]);

    struct file_node *node = call_open_legacy(path);

    if (!node)
    {

        file_write_string(FILE_STDOUT, "Directory does not exist.\n");

        return;

    }

    if (!node->walk)
    {

        file_write_string(FILE_STDOUT, "Not a directory.\n");

        return;

    }

    struct file_node *current;
    unsigned int i;

    for (i = 0; (current = node->walk(node, i)); i++)
    {

        if (current->walk)
            file_write_string(FILE_STDOUT, "d");
        else
            file_write_string(FILE_STDOUT, "-");

        file_write_string(FILE_STDOUT, "rwxrwxrwx ");
        file_write_dec(FILE_STDOUT, current->length);
        file_write_string(FILE_STDOUT, "\t");
        file_write_string(FILE_STDOUT, current->name);
        file_write_string(FILE_STDOUT, "\n");

    }

}

