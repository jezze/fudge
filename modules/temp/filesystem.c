#include <fudge/module.h>
#include <kernel/vfs.h>
#include "temp.h"
#include "filesystem.h"

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

    struct temp_filesystem *filesystem = (struct temp_filesystem *)self;

    if (id > 1)
        return memory_read(buffer, count, filesystem->buffers[id - 2].data, filesystem->buffers[id - 2].count, offset);

    if (id == 1)
        return memory_read(buffer, count, "../\n0\n1\n2\n3\n", 12, offset);

    return 0;

}

static unsigned int walk(struct vfs_interface *self, unsigned int id, unsigned int count, const char *path)
{

    if (!count)
        return id;

    if (memory_match(path, "../", 3))
        return walk(self, 1, count - 3, path + 3);

    if (memory_match(path, "0", 1))
        return walk(self, 2, count - 1, path + 1);

    if (memory_match(path, "1", 1))
        return walk(self, 3, count - 1, path + 1);

    if (memory_match(path, "2", 1))
        return walk(self, 4, count - 1, path + 1);

    if (memory_match(path, "3", 1))
        return walk(self, 5, count - 1, path + 1);

    return 0;

}

static unsigned int write(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct temp_filesystem *filesystem = (struct temp_filesystem *)self;

    if (id > 1)
        return filesystem->buffers[id - 2].count = memory_write(filesystem->buffers[id - 2].data, 0x1000, buffer, count, offset);

    return 0;

}

void temp_init_filesystem(struct temp_filesystem *filesystem)
{

    memory_clear(filesystem, sizeof (struct temp_filesystem));
    vfs_init_interface(&filesystem->base, 1, open, close, read, write, walk, 0);

}

