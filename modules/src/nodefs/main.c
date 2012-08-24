#include <vfs.h>
#include <modules/modules.h>
#include <nodefs/nodefs.h>

static struct nodefs_filesystem filesystem;

void nodefs_register_node(struct nodefs_node *node, char *name, struct modules_base *module, unsigned int (*read)(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer))
{

    filesystem.register_node(&filesystem, node, name, module, read, write);

}

void nodefs_unregister_node(struct nodefs_node *node)
{

    filesystem.unregister_node(&filesystem, node);

}

struct vfs_filesystem *get_filesystem()
{

    return &filesystem.base;

}

void init()
{

    nodefs_filesystem_init(&filesystem);
    vfs_register_filesystem(&filesystem.base);

}

void destroy()
{

    vfs_unregister_filesystem(&filesystem.base);

}

