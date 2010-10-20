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

        file_write(session_get_out(), "Directory does not exist.\n");

        return;

    }

    if (!node->walk)
    {

        file_write(session_get_out(), "Not a directory.\n");

        return;

    }

    struct vfs_node *current;
    unsigned int i;

    for (i = 0; (current = vfs_walk(node, i)); i++)
    {

        if (current->walk)
        {

            file_write(session_get_out(), "drwxrwxrwx ");
            file_write(session_get_out(), current->name);

        }

        else
        {

            file_write(session_get_out(), "-rwxrwxrwx ");
            file_write(session_get_out(), current->name);

        }

        file_write(session_get_out(), "\n");

    }

}

