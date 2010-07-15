#include <vfs.h>
#include <call.h>

void main(int argc, char *argv[])
{

    if (argc != 2)
        return;

    vfs_node_t *node = call_vfs_find("stdout");

    if (!node)
        return;

    vfs_write(node, 0, 0, argv[1]);

}

