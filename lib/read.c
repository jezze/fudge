#include <lib/call.h>
#include <lib/read.h>
#include <lib/vfs.h>

char read_char()
{

    struct vfs_node *node = call_open("dev/stdin");

    if (!node)
        return 0;

    char c;

    while (!(vfs_read(node, 0, 1, &c)));

    return c;

}

