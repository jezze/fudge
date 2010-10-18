#include <lib/memory.h>
#include <lib/string.h>
#include <lib/vfs.h>

void vfs_open(struct vfs_node *node)
{

    if (node->open)
        return node->open(node);

}

void vfs_close(struct vfs_node *node)
{

    if (node->close)
        return node->close(node);

}

unsigned int vfs_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    return (node->read) ? node->read(node, offset, count, buffer) : 0;

}

unsigned int vfs_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    return (node->write) ? node->write(node, offset, count, buffer) : 0;

}

struct vfs_node *vfs_walk(struct vfs_node *node, unsigned int index)
{

    return (node->walk) ? node->walk(node, index) : 0;

}

struct vfs_node *vfs_find(struct vfs_node *node, char *path)
{

    unsigned int index = string_index(path, '/');
    unsigned int length = string_length(path);

    if (!index)
        return node;

    struct vfs_node *current;
    unsigned int i;

    for (i = 0; (current = vfs_walk(node, i)); i++)
    {

        if (!memory_compare(path, current->name, index))
            return (index == length) ? current : vfs_find(current, path + index + 1);

    }

    return 0;

}

