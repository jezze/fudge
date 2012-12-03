#include <memory.h>
#include <vfs.h>

static struct vfs_interface interface;

static unsigned int read(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    if (id > 0x00000001)
        return memory_read(buffer, count, "../\n", 4, offset);

    if (id == 1)
        return memory_read(buffer, count, "../\nnodefs/\nramdisk/\ntty/\nvideo/\n", 33, offset);

    return 0;

}

static unsigned int write(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int walk(struct vfs_interface *self, unsigned int id, unsigned int count, char *path)
{

    if (!count)
        return id;

    if (memory_match(path, "../", 3))
        return walk(self, 1, count - 3, path + 3);

    if (memory_match(path, "block/", 6))
        return walk(self, 2, count - 6, path + 6);

    if (memory_match(path, "net/", 4))
        return walk(self, 3, count - 4, path + 4);

    if (memory_match(path, "nodefs/", 7))
        return walk(self, 4, count - 7, path + 7);

    if (memory_match(path, "ramdisk/", 8))
        return walk(self, 5, count - 8, path + 8);

    if (memory_match(path, "tty/", 4))
        return walk(self, 6, count - 4, path + 4);

    if (memory_match(path, "video/", 6))
        return walk(self, 7, count - 6, path + 6);

    return 0;

}

struct vfs_interface *root_setup()
{

    memory_clear(&interface, sizeof (struct vfs_interface));

    vfs_init_interface(&interface, 1, "root", 0, 0, read, write, walk, 0);

    return &interface;

}

