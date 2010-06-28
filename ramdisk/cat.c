#include <lib/types.h>
#include <lib/string.h>
#include <kernel/syscall.h>
#include <kernel/vfs.h>

void main(int argc, char *argv[])
{

    vfs_node_t *node = call_vfs_find("about.txt");

    if (!node)
        return;

    char buffer[400];

    uint32_t size = call_vfs_read(node, buffer);
        
    buffer[399] = '\0';

    call_puts(buffer);
    call_puts("\n");

}

