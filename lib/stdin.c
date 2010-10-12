#include <lib/call.h>
#include <lib/stdin.h>
#include <lib/vfs.h>

char stdin_read_single()
{

    struct vfs_node *node = call_open("dev/stdin");

    if (!node)
        return 0;

    char c;

    while (!(vfs_read(node, 0, 1, &c)));

    return c;

}

