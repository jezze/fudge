#include <call.h>
#include <vfs.h>

void main(int argc, char *argv[])
{

    if (argc != 2)
        return;

    struct vfs_node *node = call_vfs_find(argv[1]);

    if (!node)
        return;

    char buffer[2000];

    unsigned int size = vfs_read(node, 0, 2000, buffer);

    struct vfs_node *out = call_vfs_find("stdout");

    vfs_write(out, 0, size, buffer);

}

