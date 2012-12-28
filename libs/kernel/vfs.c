#include <fudge/memory.h>
#include "vfs.h"

static struct vfs_interface root;

static unsigned int read(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    if (id > 0x00000001)
        return memory_read(buffer, count, "../\n", 4, offset);

    if (id == 0x00000001)
        return memory_read(buffer, count, "../\nconfig/\nramdisk/\nsystem/\nnodefs/\n", 37, offset);

    return 0;

}

static unsigned int walk(struct vfs_interface *self, unsigned int id, unsigned int count, const char *path)
{

    if (!count)
        return id;

    if (memory_match(path, "../", 3))
        return walk(self, 1, count - 3, path + 3);

    if (memory_match(path, "config/", 7))
        return walk(self, 2, count - 7, path + 7);

    if (memory_match(path, "ramdisk/", 8))
        return walk(self, 3, count - 8, path + 8);

    if (memory_match(path, "system/", 7))
        return walk(self, 4, count - 7, path + 7);

    if (memory_match(path, "nodefs/", 7))
        return walk(self, 5, count - 7, path + 7);

    return 0;

}

void vfs_init_interface(struct vfs_interface *interface, unsigned int rootid, char *name, void (*open)(struct vfs_interface *self, unsigned int id), void (*close)(struct vfs_interface *self, unsigned int id), unsigned int (*read)(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*walk)(struct vfs_interface *self, unsigned int id, unsigned int count, const char *path), unsigned int (*get_physical)(struct vfs_interface *self, unsigned int id))
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

struct vfs_interface *vfs_setup()
{

    vfs_init_interface(&root, 1, "root", 0, 0, read, 0, walk, 0);

    return &root;

}

