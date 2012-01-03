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

struct vfs_node *vfs_find(char *viewname, char *nodename)
{

    if (!string_length(viewname) || !string_length(nodename))
        return 0;

    unsigned int i;

    for (i = 0; i < VFS_FILESYSTEM_SLOTS; i++)
    {

        if (!filesystems[i])
            continue;

        struct vfs_view *view = filesystems[i]->find_view(filesystems[i], viewname);

        if (!view)
            continue;

        struct vfs_node *node = view->find_node(view, nodename);

        if (node)
            return node;

    }

    return 0;

}

void vfs_node_init(struct vfs_node *node, unsigned int id, void (*open)(struct vfs_node *self), void (*close)(struct vfs_node *self), unsigned int (*read)(struct vfs_node *self, unsigned int count, void *buffer), unsigned int (*write)(struct vfs_node *self, unsigned int count, void *buffer))
{

    memory_clear(node, sizeof (struct vfs_node));

    node->id = id;
    node->open = open;
    node->close = close;
    node->read = read;
    node->write = write;

}

void vfs_view_init(struct vfs_view *view, char *name, struct vfs_node *(*find_node)(struct vfs_view *self, char *name), struct vfs_node *(*walk)(struct vfs_view *self, unsigned int index), char *(*get_name)(struct vfs_view *self, struct vfs_node *node))
{

    memory_clear(view, sizeof (struct vfs_view));

    string_write(view->name, name);
    view->find_node = find_node;
    view->walk = walk;
    view->get_name = get_name;

}

void vfs_filesystem_init(struct vfs_filesystem *filesystem, struct vfs_view *(find_view)(struct vfs_filesystem *self, char *name))
{

    memory_clear(filesystem, sizeof (struct vfs_filesystem));

    filesystem->find_view = find_view;

}

