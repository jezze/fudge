#include <vfs.h>
#include <call.h>

void main(int argc, char *argv[])
{

    vfs_node_t *node;

    unsigned int i;

    for (i = 0; (node = call_vfs_walk(i)); i++)
    {

        call_puts(node->name);
        call_puts("\n");

    }

}

