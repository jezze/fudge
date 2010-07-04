#include <kernel/syscall.h>
#include <kernel/vfs.h>

void main(int argc, char *argv[])
{

    if (argc != 2)
        return;

    vfs_node_t *node = call_vfs_find(argv[1]);

    if (!node)
        return;

    char buffer[400];

    unsigned int size = node->read(node, 0, 400, buffer);
        
    buffer[399] = '\0';

    call_puts(buffer);
    call_puts("\n");

}

