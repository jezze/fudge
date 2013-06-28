#include <fudge/module.h>
#include <fudge/define.h>
#include <fudge/data/circular.h>
#include <kernel/vfs.h>
#include "temp.h"
#include "filesystem.h"

static struct circular_stream buffers[TEMP_BUFFER_SLOTS];

static unsigned int open(struct vfs_protocol *self, unsigned int id)
{

    return id;

}

static unsigned int close(struct vfs_protocol *self, unsigned int id)
{

    return id;

}

static unsigned int read(struct vfs_protocol *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    if (id > 1)
        return circular_stream_read(&buffers[id - 2], count, buffer);

    if (id == 1)
        return memory_read(buffer, count, "../\n0\n1\n2\n3\n4\n5\n6\n7\n", 20, offset);

    return 0;

}

static unsigned int walk(struct vfs_protocol *self, unsigned int id, unsigned int count, const char *path)
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

    if (memory_match(path, "4", 1))
        return walk(self, 6, count - 1, path + 1);

    if (memory_match(path, "5", 1))
        return walk(self, 7, count - 1, path + 1);

    if (memory_match(path, "6", 1))
        return walk(self, 8, count - 1, path + 1);

    if (memory_match(path, "7", 1))
        return walk(self, 9, count - 1, path + 1);

    return 0;

}

static unsigned int write(struct vfs_protocol *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    if (id > 1)
        return circular_stream_write(&buffers[id - 2], count, buffer);

    return 0;

}

void temp_init_filesystem(struct temp_filesystem *filesystem)
{

    memory_clear(filesystem, sizeof (struct temp_filesystem));
    vfs_init_protocol(&filesystem->base, 1, 0, open, close, read, write, walk, 0);

}

