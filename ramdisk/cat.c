#include <lib/types.h>
#include <lib/cbuffer.h>
#include <lib/stack.h>
#include <lib/string.h>
#include <kernel/screen.h>
#include <kernel/vfs.h>

void main(uint32_t argc, char *argv[])
{

    if (argc > 1)
    {

        vfs_node_t *node = vfs_find(fsRoot, argv[1]);

        if (!node)
            return;

        char buffer[400];

        uint32_t size = vfs_read(node, 0, 400, buffer);
        
        uint32_t i;

        for (i = 0; i < size; i++)
            screen_putc(buffer[i]);

        screen_putc('\n');

    }

}

