#include <lib/types.h>
#include <lib/cbuffer.h>
#include <lib/stack.h>
#include <lib/string.h>
#include <kernel/regs.h>
#include <kernel/screen.h>
#include <kernel/pit.h>
#include <kernel/kbd.h>
#include <kernel/vfs.h>
#include <kernel/shell.h>

uint32_t main(uint32_t argc, char *argv[])
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

