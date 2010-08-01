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

struct vfs_node *vfs_find(struct vfs_node *node, char *name)
{

    unsigned int i;
    struct vfs_node *current;

    int index = string_offset(name, '/');

    if (index >= 0)
    {

        name[index] = '\0';

        for (i = 0; (current = vfs_walk(node, i)); i++)
        {

            if (!string_compare(name, current->name))
                return vfs_find(current, name + index + 1);

        }

    }

    else
    {

        for (i = 0; (current = vfs_walk(node, i)); i++)
        {

            if (!string_compare(name, current->name))
                return current;

        }

        return 0;

    }

}

