#include <call.h>
#include <file.h>

struct file_node *get_cwd()
{

    struct file_node *node = call_open("/dev/cwd");

    char buffer[256];
    node->read(node, 0, 256, buffer);

    return call_open(buffer);

}


void main(int argc, char *argv[])
{

    struct file_node *node = get_cwd();

    if (argc == 2)
        node = file_find(node, argv[1]);

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

    struct file_node *current;
    unsigned int i;

    for (i = 0; (current = file_walk(node, i)); i++)
    {

        if (current->walk)
            file_write_string2(FILE_STDOUT, "d");
        else
            file_write_string2(FILE_STDOUT, "-");

        file_write_string2(FILE_STDOUT, "rwxrwxrwx ");
        file_write_dec2(FILE_STDOUT, current->length);
        file_write_string2(FILE_STDOUT, "\t");
        file_write_string2(FILE_STDOUT, current->name);
        file_write_string2(FILE_STDOUT, "\n");

    }

}

