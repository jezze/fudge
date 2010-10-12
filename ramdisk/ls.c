#include <call.h>
#include <vfs.h>
#include <write.h>

void main(int argc, char *argv[])
{

    struct vfs_node *node;

    if (argc == 2)
        node = call_open(argv[1]);
    else
        node = call_open(".");

    if (!node)
    {

        write_string("Directory does not exist.\n");

        return;

    }

    if (!node->walk)
    {

        write_string("Not a directory.\n");

        return;

    }

    struct vfs_node *current;

    unsigned int i;

    for (i = 0; (current = vfs_walk(node, i)); i++)
    {

        if (current->walk)
        {

            write_string("D ");
            write_string(current->name);

        }

        else
        {

            write_string("F ");
            write_string(current->name);

        }

        write_string("\n");

    }

}

