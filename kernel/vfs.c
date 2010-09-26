#include <lib/cbuffer.h>
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

    if (index < root.length)
        return rootEntries[index];
    else
        return 0;

}

static unsigned int vfs_root_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    rootEntries[root.length] = (struct vfs_node *)buffer;
    root.length++;

    return count;

}

struct vfs_node *vfs_init(unsigned int location)
{

    string_copy(root.name, "root");
    root.inode = 0;
    root.length = 0;
    root.open = 0;
    root.close = 0;
    root.read = 0;
    root.write = vfs_root_write;
    root.walk = vfs_root_walk;

    vfs_write(&root, 0, 1, dev_init());
    vfs_write(&root, 0, 1, initrd_init(location));

    return &root;

}

