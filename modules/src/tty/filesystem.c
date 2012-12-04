#include <memory.h>
#include <vfs.h>
#include <base/base.h>
#include <tty/tty.h>

static unsigned int read_cwd(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct tty_filesystem *filesystem = (struct tty_filesystem *)self;

    return memory_read(buffer, count, filesystem->driver->cwdname, filesystem->driver->cwdcount, offset);

}

static unsigned int read_root(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    return memory_read(buffer, count, "../\ncwd\n", 8, offset);

}

static unsigned int read(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    if (id == 1)
        return read_root(self, id, offset, count, buffer);

    if (id == 4)
        return read_cwd(self, id, offset, count, buffer);

    return 0;

}

static unsigned int write_cwd(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct tty_filesystem *filesystem = (struct tty_filesystem *)self;

    filesystem->driver->cwdcount = count;

    return memory_write(filesystem->driver->cwdname, TTY_CWD_SIZE, buffer, count, offset);

}

static unsigned int write(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    if (id == 4)
        return write_cwd(self, id, offset, count, buffer);

    return 0;

}

static unsigned int walk(struct vfs_interface *self, unsigned int id, unsigned int count, const char *path)
{

    if (!count)
        return id;

    if (memory_match(path, "cwd", 3))
        return walk(self, 4, count - 3, path + 3);

    return 0;

}

void tty_init_filesystem(struct tty_filesystem *filesystem, struct tty_driver *driver)
{

    memory_clear(filesystem, sizeof (struct tty_filesystem));

    vfs_init_interface(&filesystem->base, 1, "tty", 0, 0, read, write, walk, 0);

    filesystem->driver = driver;

}

