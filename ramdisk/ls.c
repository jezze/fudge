#include <lib/types.h>
#include <lib/cbuffer.h>
#include <lib/stack.h>
#include <lib/string.h>
#include <kernel/screen.h>
#include <kernel/vfs.h>

void main()
{

    uint32_t i;
    vfs_node_t *node;

    for (i = 0; (node = vfs_walk(fsRoot, i)); i++)
    {

        if (node->flags == VFS_DIRECTORY)
            screen_puts("/");

        screen_puts(node->name);
        screen_puts("\n");

    }

}

