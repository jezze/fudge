#include <lib/memory.h>
#include <lib/string.h>
#include <lib/vfs.h>
#include <kernel/dev.h>
#include <kernel/initrd.h>
#include <kernel/vfs.h>

struct vfs_node root;
struct vfs_node *rootEntries[32];

static struct vfs_node *vfs_root_walk(struct vfs_node *node, unsigned int index)
{

    if (index < node->length)
        return rootEntries[index];
    else
        return 0;

}

static unsigned int vfs_root_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    rootEntries[offset] = (struct vfs_node *)buffer;
    node->length++;

    return count;

}

struct vfs_node *vfs_init()
{

    memory_set(&root, 0, sizeof (struct vfs_node));
    string_copy(root.name, "root");
    root.write = vfs_root_write;
    root.walk = vfs_root_walk;

    return &root;

}

