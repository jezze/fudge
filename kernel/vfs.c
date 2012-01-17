#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/vfs.h>

static struct vfs_filesystem *filesystems[VFS_FILESYSTEM_SLOTS];

void vfs_register_filesystem(struct vfs_filesystem *filesystem)
{

    unsigned int i;

    for (i = 0; i < VFS_FILESYSTEM_SLOTS; i++)
    {

        if (filesystems[i])
            continue;

        filesystems[i] = filesystem;

        return;

    }

}

struct vfs_filesystem *vfs_get_filesystem(unsigned int index)
{

    if (!filesystems[index])
        return 0;

    return filesystems[index];

}

struct vfs_node *vfs_find(char *path)
{

    if (!string_length(path))
        return 0;

    unsigned int i;

    for (i = 0; i < VFS_FILESYSTEM_SLOTS; i++)
    {

        if (!filesystems[i])
            continue;

        struct vfs_node *node = filesystems[i]->find_node(filesystems[i], path);

        if (node)
            return node;

    }

    return 0;

}

void vfs_node_init(struct vfs_node *node, char *name, unsigned int id, void (*open)(struct vfs_node *self), void (*close)(struct vfs_node *self), unsigned int (*read)(struct vfs_node *self, unsigned int count, void *buffer), unsigned int (*write)(struct vfs_node *self, unsigned int count, void *buffer))
{

    memory_clear(node, sizeof (struct vfs_node));

    node->name = name;
    node->id = id;
    node->open = open;
    node->close = close;
    node->read = read;
    node->write = write;

}

void vfs_filesystem_init(struct vfs_filesystem *filesystem, struct vfs_node *(*find_node)(struct vfs_filesystem *self, char *name), struct vfs_node *(*walk)(struct vfs_filesystem *self, unsigned int index))
{

    memory_clear(filesystem, sizeof (struct vfs_filesystem));

    filesystem->find_node = find_node;
    filesystem->walk = walk;

}

