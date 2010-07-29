#include <call.h>
#include <vfs.h>

void main(int argc, char *argv[])
{

    struct vfs_node *node;
    struct vfs_node *current;

    if (argc == 2)
    {

        node = call_vfs_find(argv[1]);

        if (!node)
        {

            call_puts("Directory does not exist.\n");
            return;

        }

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

    else
    {


        unsigned int i;

        for (i = 0; (current = call_vfs_walk(i)); i++)
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

}

