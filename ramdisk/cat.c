#include <kernel/call.h>
#include <kernel/vfs.h>

void main(int argc, char *argv[])
{

    if (argc != 2)
        return;

    vfs_node_t *node = call_vfs_find(argv[1]);

    if (!node)
        return;

    char buffer[2000];

    unsigned int size = node->read(node, 0, 2000, buffer);
    
    if (size == 2000)
        buffer[size - 1] = '\0';
    else
        buffer[size] = '\0';

    call_puts(buffer);

}

