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

    if (node->read)
        return node->read(node, offset, count, buffer);
    else
        return 0;

}

unsigned int vfs_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    if (node->write)
        return node->write(node, offset, count, buffer);
    else
        return 0;

}

struct vfs_node *vfs_walk(struct vfs_node *node, unsigned int index)
{

    if (node->walk)
        return node->walk(node, index);
    else
        return 0;

}

struct vfs_node *vfs_find(struct vfs_node *node, char *path)
{

    unsigned int i;
    struct vfs_node *current;

    unsigned int index = string_index(path, '/');
    unsigned int length = string_length(path);

    if (index != length)
    {

        if (length == 1)
            return node;

        if (length == index + 1)
            return node;

        for (i = 0; (current = vfs_walk(node, i)); i++)
        {

            if (!memory_compare(path, current->name, index))
                return vfs_find(current, path + index + 1);

        }

    }

    else
    {

        if (!memory_compare(path, ".", 2))
            return node;

        for (i = 0; (current = vfs_walk(node, i)); i++)
        {

            if (string_length(current->name) == index && !memory_compare(path, current->name, index))
                return current;

        }

    }

    return 0;

}

