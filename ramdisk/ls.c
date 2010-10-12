#include <call.h>
#include <stdout.h>
#include <vfs.h>

void main(int argc, char *argv[])
{

    struct vfs_node *node;

    if (argc == 2)
        node = call_open(argv[1]);
    else
        node = call_open(".");

    if (!node)
    {

        stdout_write("Directory does not exist.\n");

        return;

    }

    if (!node->walk)
    {

        stdout_write("Not a directory.\n");

        return;

    }

    struct vfs_node *current;

    unsigned int i;

    for (i = 0; (current = vfs_walk(node, i)); i++)
    {

        if (current->walk)
        {

            stdout_write("D ");
            stdout_write(current->name);

        }

        else
        {

            stdout_write("F ");
            stdout_write(current->name);

        }

        stdout_write("\n");

    }

}

