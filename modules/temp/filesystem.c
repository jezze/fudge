#include <fudge/module.h>
#include <fudge/define.h>
#include <fudge/data/circular.h>
#include <kernel/vfs.h>
#include "temp.h"
#include "filesystem.h"

#define TEMP_BUFFERS                    8

static struct circular_stream buffers[TEMP_BUFFERS];

static unsigned int backend_read(struct vfs_backend *self, unsigned int offset, unsigned int count, void *buffer)
{

    return memory_read(buffer, count, "FUDGE_TEMP", 10, offset);

}

static unsigned int backend_write(struct vfs_backend *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int parent(struct vfs_backend *backend, unsigned int id)
{

    return 1;

}

static unsigned int match(struct vfs_backend *backend)
{

    char buffer[16];

    backend->read(backend, 0, 16, buffer);

    return memory_match(buffer, "FUDGE_TEMP", 10);

}

static unsigned int open(struct vfs_backend *backend, unsigned int id)
{

    if (id > 1)
        circular_stream_clear(&buffers[id - 2]);

    return 1;

}

static unsigned int close(struct vfs_backend *backend, unsigned int id)
{

    return 1;

}

static unsigned int read(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    if (id > 1)
        return circular_stream_read(&buffers[id - 2], count, buffer);

    if (id == 1)
        return memory_read(buffer, count, "../\n0\n1\n2\n3\n4\n5\n6\n7\n", 20, offset);

    return 0;

}

static unsigned int walk(struct vfs_backend *backend, unsigned int id, unsigned int count, const char *path)
{

    if (!count)
        return id;

    if (memory_match(path, "0", 1))
        return walk(backend, 2, count - 1, path + 1);

    if (memory_match(path, "1", 1))
        return walk(backend, 3, count - 1, path + 1);

    if (memory_match(path, "2", 1))
        return walk(backend, 4, count - 1, path + 1);

    if (memory_match(path, "3", 1))
        return walk(backend, 5, count - 1, path + 1);

    if (memory_match(path, "4", 1))
        return walk(backend, 6, count - 1, path + 1);

    if (memory_match(path, "5", 1))
        return walk(backend, 7, count - 1, path + 1);

    if (memory_match(path, "6", 1))
        return walk(backend, 8, count - 1, path + 1);

    if (memory_match(path, "7", 1))
        return walk(backend, 9, count - 1, path + 1);

    return 0;

}

static unsigned int write(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    if (id > 1)
        return circular_stream_write(&buffers[id - 2], count, buffer);

    return 0;

}

void temp_init_backend(struct vfs_backend *backend)
{

    vfs_init_backend(backend, backend_read, backend_write);

}

void temp_init_protocol(struct vfs_protocol *protocol)
{

    vfs_init_protocol(protocol, 1, match, open, close, read, write, parent, walk, 0);

}

