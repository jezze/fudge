#include <call.h>
#include <vfs.h>

void main(int argc, char *argv[])
{

    struct vfs_node *node;

    if (argc == 2)
        node = call_vfs_find(argv[1]);
    else
        node = call_vfs_find("/");

    if (!node)
        return;

    struct vfs_node *current;

    unsigned int i;

    for (i = 0; (current = vfs_walk(node, i)); i++)
    {

        if (current->walk)
        {

            call_puts(current->name);
            call_puts(" [DIR]");

        }

        else
        {

            call_puts(current->name);

        }

        call_puts("\n");

    }

}

