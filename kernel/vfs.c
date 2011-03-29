#include <lib/file.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/vfs.h>

struct file_node vfsNodes[256];
unsigned int vfsNodesCount;

struct file_node *vfsRootEntries[64];

struct file_node *vfs_get_root(struct file_node *node, unsigned int index)
{

    return &vfsNodes[0];

}

static struct file_node *file_node_walk(struct file_node *node, unsigned int index)
{

    return (index < node->length) ? vfsRootEntries[index] : 0;

}

static unsigned int file_node_write(struct file_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    vfsRootEntries[offset] = (struct file_node *)buffer;
    node->length++;

    return count;

}

struct file_node *vfs_add_node(char *name, unsigned int length)
{

    struct file_node *node = &vfsNodes[vfsNodesCount];
    memory_set(node, 0, sizeof (struct file_node));
    string_copy(node->name, name);
    node->cursor = 0;
    node->length = length;

    vfsNodesCount++;

    return node;

}

void vfs_init()
{

    struct file_node *node = vfs_add_node("root", 0);
    node->walk = file_node_walk;
    node->write = file_node_write;

}

