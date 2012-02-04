#include <lib/memory.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <kernel/vfs.h>
#include <modules/nodefs/nodefs.h>

static void register_node(struct nodefs_driver *self, struct vfs_node *node)
{

    unsigned int i;

    for (i = 0; i < 128; i++)
    {

        if (!self->nodes[i])
            self->nodes[i] = node;

    }

}

static void unregister_node(struct nodefs_driver *self, struct vfs_node *node)
{

    unsigned int i;

    for (i = 0; i < 128; i++)
    {

        if (self->nodes[i] == node)
            self->nodes[i] = 0;

    }

}

void nodefs_driver_init(struct nodefs_driver *driver)
{

    modules_driver_init(&driver->base, NODEFS_DRIVER_TYPE);

    driver->register_node = register_node;
    driver->unregister_node = unregister_node;

}

