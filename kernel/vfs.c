#include <lib/memory.h>
#include <lib/string.h>
#include <lib/vfs.h>
#include <kernel/dev.h>
#include <kernel/initrd.h>
#include <kernel/vfs.h>

struct vfs_node vfsNodes[256];
unsigned int vfsNodesCount;

struct vfs_node *vfsRootEntries[32];

struct vfs_node *vfs_get_root(struct vfs_node *node, unsigned int index)
{

    return &vfsNodes[0];

}

static struct vfs_node *vfs_node_walk(struct vfs_node *node, unsigned int index)
{

    if (index < node->length)
        return vfsRootEntries[index];
    else
        return 0;

}

static unsigned int vfs_node_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    vfsRootEntries[offset] = (struct vfs_node *)buffer;
    node->length++;

    return count;

}

struct vfs_node *vfs_add_node(char *name, unsigned int length)
{

    struct vfs_node *node = &vfsNodes[vfsNodesCount];
    memory_set(node, 0, sizeof (struct vfs_node));
    string_copy(node->name, name);
    node->length = length;

    vfsNodesCount++;

    return node;

}

void vfs_init()
{

    struct vfs_node *node = vfs_add_node("root", 0);
    node->walk = vfs_node_walk;
    node->write = vfs_node_write;

}

