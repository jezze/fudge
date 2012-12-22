#include <kernel/vfs.h>
#include <base/base.h>
#include "nodefs.h"

static struct nodefs_filesystem filesystem;

void nodefs_register_node(struct nodefs_node *node, char *name, struct base_module *module, unsigned int (*read)(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer))
{

    filesystem.register_node(&filesystem, node, name, module, read, write);

}

void nodefs_unregister_node(struct nodefs_node *node)
{

    filesystem.unregister_node(&filesystem, node);

}

struct vfs_interface *get_filesystem()
{

    return &filesystem.base;

}

void init()
{

    nodefs_init_filesystem(&filesystem);

}

void destroy()
{

}

