#include <lib/types.h>
#include <lib/string.h>
#include <lib/vfs.h>

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

unsigned int vfs_read(vfs_node_t *node, unsigned int offset, unsigned int count, void *buffer)
{

    if (node->read)
        return node->read(node, offset, count, buffer);
    else
        return 0;

}

unsigned int vfs_write(vfs_node_t *node, unsigned int offset, unsigned int count, void *buffer)
{

    if (node->write)
        return node->write(node, offset, count, buffer);
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

    unsigned int i;
    vfs_node_t *current;

    for (i = 0; (current = vfs_walk(node, i)); i++)
    {

        if (!string_compare(name, current->name))
            return current;

    }

    return 0;

}

