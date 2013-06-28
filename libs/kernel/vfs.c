#include <fudge/kernel.h>
#include "vfs.h"

static struct vfs_interface *interfaces;

struct vfs_interface *vfs_get_interface()
{

    struct vfs_interface *current;

    for (current = interfaces; current; current = current->sibling)
    {

        if (current->match())
            return current;

    }

    return 0;

}

void vfs_register_interface(struct vfs_interface *interface)
{

    struct vfs_interface *current;

    if (!interfaces)
    {

        interfaces = interface;

        return;

    }

    for (current = interfaces; current; current = current->sibling)
    {

        if (current->sibling)
            continue;

        current->sibling = interface;

        return;

    }

}

void vfs_unregister_interface(struct vfs_interface *interface)
{

    struct vfs_interface *current;

    if (interfaces == interface)
    {

        interfaces = interfaces->sibling;

        return;

    }

    for (current = interfaces; current; current = current->sibling)
    {

        if (current->sibling != interface)
            continue;

        current->sibling = current->sibling->sibling;

        return;

    }

}

void vfs_init_interface(struct vfs_interface *interface, unsigned int rootid, unsigned int (*match)(), unsigned int (*open)(struct vfs_interface *self, unsigned int id), unsigned int (*close)(struct vfs_interface *self, unsigned int id), unsigned int (*read)(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*walk)(struct vfs_interface *self, unsigned int id, unsigned int count, const char *path), unsigned int (*get_physical)(struct vfs_interface *self, unsigned int id))
{

    memory_clear(interface, sizeof (struct vfs_interface));

    interface->rootid = rootid;
    interface->match = match;
    interface->open = open;
    interface->close = close;
    interface->read = read;
    interface->write = write;
    interface->walk = walk;
    interface->get_physical = get_physical;

}

void vfs_setup()
{

    interfaces = 0;

}

