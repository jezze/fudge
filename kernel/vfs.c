#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/vfs.h>

static struct vfs_filesystem *vfsFilesystems[VFS_FILESYSTEM_SLOTS];
static struct vfs_node vfsRoot;

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

struct vfs_node *vfs_find(char *view, char *name)
{

    if (name[0] != '/')
        return 0;

    if (string_length(name) == 1)
        return &vfsRoot;

    unsigned int i;

    for (i = 0; vfsFilesystems[i]; i++)
    {

        struct vfs_node *node = vfsFilesystems[i]->find_node(vfsFilesystems[i], view, name + 1);

        if (node)
            return node;

    }

    return 0;

}

static unsigned int vfs_node_read(struct vfs_node *self, unsigned int count, void *buffer)
{

    memory_set(buffer, 0, 1);

    unsigned int i;

    for (i = 0; vfsFilesystems[i]; i++)
        vfsFilesystems[i]->root->read(vfsFilesystems[i]->root, count, buffer + string_length(buffer));

    return string_length(buffer);

}

void vfs_descriptor_init(struct vfs_descriptor *descriptor, unsigned int id, struct vfs_node *node, unsigned int permissions)
{

    descriptor->id = id;
    descriptor->node = node;
    descriptor->permissions = permissions;

}

void vfs_node_init(struct vfs_node *node, unsigned int id, void (*open)(struct vfs_node *self), void (*close)(struct vfs_node *self), unsigned int (*read)(struct vfs_node *self, unsigned int count, void *buffer), unsigned int (*write)(struct vfs_node *self, unsigned int count, void *buffer))
{

    node->id = id;
    node->open = open;
    node->close = close;
    node->read = read;
    node->write = write;

}

void vfs_view_init(struct vfs_view *view, char *name, struct vfs_node *(*find_node)(struct vfs_view *self, char *name))
{

    string_write(view->name, name);
    view->find_node = find_node;

}

void vfs_filesystem_init(struct vfs_filesystem *filesystem, struct vfs_node *root, struct vfs_node *(*find_node)(struct vfs_filesystem *self, char *view, char *name), struct vfs_view *(find_view)(struct vfs_filesystem *self, char *name))
{

    filesystem->root = root;
    filesystem->find_node = find_node;
    filesystem->find_view = find_view;

}

void vfs_init()
{

    vfs_node_init(&vfsRoot, 0, 0, 0, vfs_node_read, 0);

}

