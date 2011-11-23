#include <lib/string.h>
#include <kernel/vfs.h>

static struct vfs_filesystem *vfsFilesystems[VFS_FILESYSTEM_SLOTS];

void vfs_register_filesystem(struct vfs_filesystem *filesystem)
{

    unsigned int i;

    for (i = 0; i < VFS_FILESYSTEM_SLOTS; i++)
    {

        if (vfsFilesystems[i])
            continue;

        vfsFilesystems[i] = filesystem;

        return;

    }

}

struct vfs_filesystem *vfs_get_filesystem(unsigned int index)
{

    if (!vfsFilesystems[index])
        return 0;

    return vfsFilesystems[index];

}

struct vfs_node *vfs_find(char *view, char *name)
{

    if (!string_length(name))
        return 0;

    unsigned int i;

    for (i = 0; i < VFS_FILESYSTEM_SLOTS; i++)
    {

        if (!vfsFilesystems[i])
            continue;

        struct vfs_view *v = vfsFilesystems[i]->find_view(vfsFilesystems[i], view);

        if (!v)
            continue;

        struct vfs_node *node = v->find_node(v, name);

        if (node)
            return node;

    }

    return 0;

}

void vfs_node_init(struct vfs_node *node, unsigned int id, void (*open)(struct vfs_node *self), void (*close)(struct vfs_node *self), unsigned int (*read)(struct vfs_node *self, unsigned int count, void *buffer), unsigned int (*write)(struct vfs_node *self, unsigned int count, void *buffer))
{

    node->id = id;
    node->open = open;
    node->close = close;
    node->read = read;
    node->write = write;

}

void vfs_view_init(struct vfs_view *view, char *name, struct vfs_node *(*find_node)(struct vfs_view *self, char *name), struct vfs_node *(*walk)(struct vfs_view *self, unsigned int index))
{

    string_write(view->name, name);
    view->find_node = find_node;
    view->walk = walk;

}

void vfs_filesystem_init(struct vfs_filesystem *filesystem, struct vfs_view *(find_view)(struct vfs_filesystem *self, char *name))
{

    filesystem->find_view = find_view;

}

