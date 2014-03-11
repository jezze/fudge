#include <kernel.h>
#include "resource.h"
#include "vfs.h"
#include "task.h"
#include "container.h"
#include "kernel.h"

static struct resource_iterator base;

static unsigned int base_match(struct resource_item *item)
{

    return 1;

}

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

    return (unsigned int)&base;

}

static unsigned int parent(struct vfs_backend *backend, unsigned int id)
{

    return (unsigned int)&base;

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

    return (unsigned int)&base;

}

static unsigned int close(struct vfs_backend *backend, unsigned int id)
{

    return 0;

}

static unsigned int read(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int write(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int walk(struct vfs_backend *backend, unsigned int id, unsigned int count, const char *path)
{

    if (!count)
        return id;

    return 0;

}

void vfs_setup_kernel(struct vfs_backend *backend, struct vfs_protocol *protocol)
{

    vfs_init_backend(backend, backend_read, backend_write);
    vfs_init_protocol(protocol, match, root, open, close, read, write, parent, walk, get_physical);
    resource_init_iterator(&base, base_match);

}

