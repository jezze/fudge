#include <memory.h>
#include <vfs.h>

static struct vfs_interface interface;

static unsigned int read(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    if (id > 0x00000001)
        return memory_read(buffer, count, "../\n", 4, offset);

    if (id == 1)
        return memory_read(buffer, count, "../\ndev/\nramdisk/\n", 18, offset);

    return 0;

}

static unsigned int walk(struct vfs_interface *self, unsigned int id, unsigned int count, char *path)
{

    if (!count)
        return id;

    if (memory_match(path, "../", 3))
        return walk(self, 1, count - 3, path + 3);

    if (memory_match(path, "dev/", 4))
        return walk(self, 2, count - 4, path + 4);

    if (memory_match(path, "ramdisk/", 8))
        return walk(self, 3, count - 8, path + 8);

    return 0;

}

struct vfs_interface *root_setup()
{

    memory_clear(&interface, sizeof (struct vfs_interface));

    vfs_init_interface(&interface, 1, "root", 0, 0, read, 0, walk, 0);

    return &interface;

}

