#include <lib/memory.h>
#include <lib/string.h>
#include <lib/vfs.h>
#include <kernel/dev.h>
#include <kernel/initrd.h>
#include <kernel/vfs.h>

struct vfs_node vfsNode;
struct vfs_node *vfsEntries[32];

struct vfs_node *vfs_get_root(struct vfs_node *node, unsigned int index)
{

    return &vfsNode;

}

static struct vfs_node *vfs_node_walk(struct vfs_node *node, unsigned int index)
{

    if (index < node->length)
        return vfsEntries[index];
    else
        return 0;

}

static unsigned int vfs_node_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    vfsEntries[offset] = (struct vfs_node *)buffer;
    node->length++;

    return count;

}

void vfs_init()
{

    memory_set(&vfsNode, 0, sizeof (struct vfs_node));
    string_copy(vfsNode.name, "root");
    vfsNode.walk = vfs_node_walk;
    vfsNode.write = vfs_node_write;

}

