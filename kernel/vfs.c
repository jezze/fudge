#include <lib/types.h>
#include <kernel/vfs.h>

vfs_node_t *fsRoot = 0;

void vfs_open(vfs_node_t *node)
{

    if (node->open)
        return node->open(node);

}

void vfs_close(vfs_node_t *node)
{

    if (node->close)
        return node->close(node);

}

unsigned int vfs_read(vfs_node_t *node, unsigned int offset, unsigned int size, char *buffer)
{

    if (node->read)
        return node->read(node, offset, size, buffer);
    else
        return 0;

}

unsigned int vfs_write(vfs_node_t *node, unsigned int offset, unsigned int size, char *buffer)
{

    if (node->write)
        return node->write(node, offset, size, buffer);
    else
        return 0;

}

vfs_node_t *vfs_walk(vfs_node_t *node, unsigned int index)
{

    if (node->walk)
        return node->walk(node, index);
    else
        return 0;

}

vfs_node_t *vfs_find(vfs_node_t *node, char *name)
{

    if (node->find)
        return node->find(node, name);
    else
        return 0;

}

