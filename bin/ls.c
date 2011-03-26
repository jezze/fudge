#include <call.h>
#include <file.h>
#include <session.h>

void main(int argc, char *argv[])
{

    struct vfs_node *node = session_get_cwd();

    if (argc == 2)
        node = file_find(node, argv[1]);

    if (!node)
    {

        file_write_string(session_get_out(), "Directory does not exist.\n");

        return;

    }

    if (!node->walk)
    {

        file_write_string(session_get_out(), "Not a directory.\n");

        return;

    }

    struct vfs_node *current;
    unsigned int i;

    for (i = 0; (current = file_walk(node, i)); i++)
    {

        if (current->walk)
            file_write_string(session_get_out(), "d");
        else
            file_write_string(session_get_out(), "-");

        file_write_string(session_get_out(), "rwxrwxrwx ");
        file_write_dec(session_get_out(), current->length);
        file_write_string(session_get_out(), "\t");
        file_write_string(session_get_out(), current->name);
        file_write_string(session_get_out(), "\n");

    }

}

