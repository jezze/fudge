#include <lib/types.h>
#include <kernel/vfs.h>

vfs_node_t *fsRoot = 0;

uint32_t vfs_read(vfs_node_t *node, uint32_t offset, uint32_t size, char *buffer)
{

    if (node->read != 0)
        return node->read(node, offset, size, buffer);
    else
        return 0;

}

uint32_t vfs_write(vfs_node_t *node, uint32_t offset, uint32_t size, char *buffer)
{

    if (node->write != 0)
        return node->write(node, offset, size, buffer);
    else
        return 0;

}

void vfs_open(vfs_node_t *node, uint8_t read, uint8_t write)
{

    if (node->open != 0)
        return node->open(node);

}

void vfs_close(vfs_node_t *node)
{

    if (node->close != 0)
        return node->close(node);

}

vfs_node_t *vfs_directory_read(vfs_node_t *node, uint32_t index)
{

    if (node->readdir != 0)
        return node->readdir(node, index);
    else
        return 0;

}

vfs_node_t *vfs_directory_find(vfs_node_t *node, char *name)
{

    if (node->finddir != 0)
        return node->finddir(node, name);
    else
        return 0;

}

