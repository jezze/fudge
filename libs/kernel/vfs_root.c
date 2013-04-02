#include <fudge/kernel.h>
#include "vfs.h"
#include "vfs_root.h"

static struct vfs_interface root;

static unsigned int open(struct vfs_interface *self, unsigned int id)
{

    return id;

}

static unsigned int close(struct vfs_interface *self, unsigned int id)
{

    return id;

}

static unsigned int read(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    if (id > 1)
        return memory_read(buffer, count, "../\n", 4, offset);

    if (id == 1)
        return memory_read(buffer, count, "../\nconfig/\nramdisk/\nsystem/\ntemp/\n", 35, offset);

    return 0;

}

static unsigned int write(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

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

    if (memory_match(path, "temp/", 5))
        return walk(self, 5, count - 5, path + 5);

    return 0;

}

struct vfs_interface *vfs_root_setup()
{

    vfs_init_interface(&root, 1, open, close, read, write, walk, 0);

    return &root;

}

