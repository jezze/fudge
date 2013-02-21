#include <fudge/kernel.h>
#include "vfs.h"

void vfs_init_interface(struct vfs_interface *interface, unsigned int rootid, char *name, unsigned int (*open)(struct vfs_interface *self, unsigned int id), unsigned int (*close)(struct vfs_interface *self, unsigned int id), unsigned int (*read)(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*walk)(struct vfs_interface *self, unsigned int id, unsigned int count, const char *path), unsigned int (*get_physical)(struct vfs_interface *self, unsigned int id))
{

    memory_clear(interface, sizeof (struct vfs_interface));

    interface->rootid = rootid;
    interface->open = open;
    interface->close = close;
    interface->read = read;
    interface->write = write;
    interface->walk = walk;
    interface->get_physical = get_physical;

}

