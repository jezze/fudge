#include <lib/types.h>
#include <lib/string.h>
#include <kernel/syscall.h>
#include <kernel/vfs.h>

void main()
{

    vfs_node_t *node;

    unsigned int i;

    for (i = 1; (node = (vfs_node_t *)call_send(SYSCALL_VFS_WALK, &i)); i++)
    {

        if (node->flags == VFS_DIRECTORY)
            call_send(SYSCALL_SCREEN_PUTS, "/");

        call_send(SYSCALL_SCREEN_PUTS, node->name);
        call_send(SYSCALL_SCREEN_PUTS, "\n");

    }

}

