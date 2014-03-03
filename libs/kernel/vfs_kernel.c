#include <kernel.h>
#include "vfs.h"
#include "task.h"
#include "container.h"
#include "kernel.h"

static unsigned int backend_read(struct vfs_backend *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int backend_write(struct vfs_backend *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int root(struct vfs_backend *backend)
{

    return 1;

}

static unsigned int parent(struct vfs_backend *backend, unsigned int id)
{

    return 1;

}

static unsigned int match(struct vfs_backend *backend)
{

    return 0;

}

static unsigned long get_physical(struct vfs_backend *backend, unsigned int id)
{

    return 0;

}

static unsigned int open(struct vfs_backend *backend, unsigned int id)
{

    return id;

}

static unsigned int close(struct vfs_backend *backend, unsigned int id)
{

    return 0;

}

static unsigned int read(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    if (id == 1)
    {

        return memory_read(buffer, count, "../\ncontainer/\ntask/\nbackend/\n", 30, offset);

    }

    if (id > 1)
    {

        return memory_read(buffer, count, "../\n0/\n1/\n2/\n", 13, offset);

    }

    return 0;

}

static unsigned int write(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int walk(struct vfs_backend *backend, unsigned int id, unsigned int count, const char *path)
{

    unsigned int n = vfs_findnext(count, path);

    if (!count)
        return id;

    if (id == 1)
    {

        if (memory_match(path, "container/", 10))
            return walk(backend, 2, count - n, path + n);

        if (memory_match(path, "task/", 5))
            return walk(backend, 3, count - n, path + n);

        if (memory_match(path, "backend/", 8))
            return walk(backend, 4, count - n, path + n);

    }

    return 0;

}

void vfs_init_kernel(struct vfs_backend *backend, struct vfs_protocol *protocol)
{

    memory_clear(backend, sizeof (struct vfs_backend));
    vfs_init_backend(backend, backend_read, backend_write);

    memory_clear(protocol, sizeof (struct vfs_protocol));
    vfs_init_protocol(protocol, match, root, open, close, read, write, parent, walk, get_physical);

}

