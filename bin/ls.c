#include <call.h>
#include <file.h>
#include <session.h>
#include <vfs.h>

void main(int argc, char *argv[])
{

    struct vfs_node *node = session_get_location();

    if (argc == 2)
        node = vfs_find(node, argv[1]);

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

    for (i = 0; (current = vfs_walk(node, i)); i++)
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

