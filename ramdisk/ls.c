#include <call.h>
#include <vfs.h>

void main(int argc, char *argv[])
{

    struct vfs_node *node;

    unsigned int i;

    for (i = 0; (node = call_vfs_walk(i)); i++)
    {

        call_puts(node->name);
        call_puts("\n");

    }

}

