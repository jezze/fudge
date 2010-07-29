#include <call.h>
#include <vfs.h>

void main(int argc, char *argv[])
{

    struct vfs_node *node;

    unsigned int i;

    for (i = 0; (node = call_vfs_walk(i)); i++)
    {

        if (node->walk)
        {

            call_puts("/");
            call_puts(node->name);

        }

        else
        {

            call_puts(node->name);

        }

        call_puts("\n");

    }

}

