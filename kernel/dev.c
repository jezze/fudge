#include <lib/call.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <lib/vfs.h>
#include <kernel/dev.h>

struct vfs_node devNode;
struct vfs_node *devEntries[32];

static struct vfs_node *dev_node_walk(struct vfs_node *node, unsigned int index)
{

    if (index < node->length)
        return devEntries[index];
    else
        return 0;

}

static unsigned int dev_node_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    devEntries[offset] = (struct vfs_node *)buffer;
    node->length++;

    return count;

}

void dev_init()
{

    memory_set(&devNode, 0, sizeof (struct vfs_node));
    string_copy(devNode.name, "dev");
    devNode.walk = dev_node_walk;
    devNode.write = dev_node_write;

    struct vfs_node *root = call_open(".");
    vfs_write(root, root->length, 1, &devNode);

}

