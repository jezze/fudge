#include <call.h>
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

        call_puts("Directory does not exist.\n");

        return;

    }

    if (!node->walk)
    {

        call_puts("Not a directory.\n");

        return;

    }

    struct vfs_node *current;

    unsigned int i;

    for (i = 0; (current = vfs_walk(node, i)); i++)
    {

        if (current->walk)
        {

            call_puts("D ");
            call_puts(current->name);

        }

        else
        {

            call_puts("F ");
            call_puts(current->name);

        }

        call_puts("\n");

    }

}

