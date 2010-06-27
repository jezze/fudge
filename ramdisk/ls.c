#include <lib/types.h>
#include <lib/string.h>
#include <kernel/syscall.h>
#include <kernel/vfs.h>

void main()
{

    vfs_node_t *node;

    unsigned int i;

    for (i = 0; i < 4; i++)
    {

        node = call_vfs_walk(i);

        if (node->flags == VFS_DIRECTORY)
            call_puts("/");

        call_puts(node->name);
        call_puts("\n");

    }

}

