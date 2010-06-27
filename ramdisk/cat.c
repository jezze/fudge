#include <lib/types.h>
#include <kernel/screen.h>
#include <kernel/vfs.h>

void main()
{

    vfs_node_t *node = vfs_find(fsRoot, "about.txt");

    if (!node)
        return;

    char buffer[400];

    uint32_t size = vfs_read(node, 0, 400, buffer);
        
    uint32_t i;

    for (i = 0; i < size; i++)
        screen_putc(buffer[i]);

    screen_putc('\n');

}

