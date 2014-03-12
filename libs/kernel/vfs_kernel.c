#include <kernel.h>
#include "resource.h"
#include "vfs.h"
#include "task.h"
#include "container.h"
#include "kernel.h"

enum
{

    VFS_KERNEL_ROOT                     = 1,
    VFS_KERNEL_VFS                      = 2

};

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

    return VFS_KERNEL_ROOT;

}

static unsigned int parent(struct vfs_backend *backend, unsigned int id)
{

    return VFS_KERNEL_ROOT;

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

    if (id == VFS_KERNEL_ROOT)
        return memory_read(buffer, count, "../\nvfs/\n", 9, offset);

    if (id == VFS_KERNEL_VFS)
        return memory_read(buffer, count, "../\n", 4, offset);

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

    if (id == VFS_KERNEL_ROOT)
    {

        if (memory_match(path, "vfs/", 4))
            return walk(backend, VFS_KERNEL_VFS, count - 4, path + 4);

    }

    return 0;

}

void vfs_setup_kernel(struct vfs_backend *backend, struct vfs_protocol *protocol)
{

    vfs_init_backend(backend, backend_read, backend_write);
    vfs_init_protocol(protocol, match, root, open, close, read, write, parent, walk, get_physical);
    resource_register_item(&backend->resource);
    resource_register_item(&protocol->resource);
    resource_init_iterator(&base, base_match);

}

