#include <kernel.h>
#include "resource.h"
#include "vfs.h"
#include "task.h"
#include "container.h"
#include "kernel.h"

enum vfs_kernel_entry_type
{

    ENTRY_TYPE_NULL                     = 0x00000000,
    ENTRY_TYPE_ROOT                     = 0x00010000,
    ENTRY_TYPE_VFS                      = 0x00020000,
    ENTRY_TYPE_VFSBACKENDS              = 0x00030000,
    ENTRY_TYPE_VFSPROTOCOLS             = 0x00040000

};

struct vfs_kernel_entry
{

    unsigned int (*read)(unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*walk)(unsigned int count, const char *path);

};

static unsigned int entry_root_read(unsigned int offset, unsigned int count, void *buffer)
{

    return memory_read(buffer, count, "../\nvfs/\n", 9, offset);

}

static unsigned int entry_root_walk(unsigned int count, const char *path)
{

    if (memory_match(path, "vfs/", count))
        return ENTRY_TYPE_VFS >> 16;

    return 0;

}

static unsigned int entry_vfs_read(unsigned int offset, unsigned int count, void *buffer)
{

    return memory_read(buffer, count, "../\nbackends/\nprotocols/\n", 25, offset);

}

static unsigned int entry_vfs_walk(unsigned int count, const char *path)
{

    if (memory_match(path, "backends/", count))
        return ENTRY_TYPE_VFSBACKENDS >> 16;

    if (memory_match(path, "protocols/", count))
        return ENTRY_TYPE_VFSPROTOCOLS >> 16;

    return 0;

}

static unsigned int entry_vfsbackends_read(unsigned int offset, unsigned int count, void *buffer)
{

    return memory_read(buffer, count, "../\n", 4, offset);

}

static unsigned int entry_vfsbackends_walk(unsigned int count, const char *path)
{

    return 0;

}

static unsigned int entry_vfsprotocols_read(unsigned int offset, unsigned int count, void *buffer)
{

    return memory_read(buffer, count, "../\n", 4, offset);

}

static unsigned int entry_vfsprotocols_walk(unsigned int count, const char *path)
{

    return 0;

}

static struct vfs_kernel_entry entries[] = {
    {0, 0},
    {entry_root_read, entry_root_walk},
    {entry_vfs_read, entry_vfs_walk},
    {entry_vfsbackends_read, entry_vfsbackends_walk},
    {entry_vfsprotocols_read, entry_vfsprotocols_walk}
};

static unsigned int backend_read(struct vfs_backend *self, unsigned int offset, unsigned int count, void *buffer)
{

    return memory_read(buffer, count, "FUDGE_INTERNAL", 14, offset);

}

static unsigned int backend_write(struct vfs_backend *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int root(struct vfs_backend *backend)
{

    return ENTRY_TYPE_ROOT >> 16;

}

static unsigned int parent(struct vfs_backend *backend, unsigned int id)
{

    return ENTRY_TYPE_ROOT >> 16;

}

static unsigned int match(struct vfs_backend *backend)
{

    char buffer[14];

    if (backend->read(backend, 0, 14, buffer) < 14)
        return 0;

    return memory_match(buffer, "FUDGE_INTERNAL", 14);

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

    return entries[id].read(offset, count, buffer);

}

static unsigned int write(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int walk(struct vfs_backend *backend, unsigned int id, unsigned int count, const char *path)
{

    if (!count)
        return id;

    return entries[id].walk(count, path);

}

void vfs_setup_kernel(struct vfs_backend *backend, struct vfs_protocol *protocol)
{

    vfs_init_backend(backend, backend_read, backend_write);
    vfs_init_protocol(protocol, match, root, open, close, read, write, parent, walk, get_physical);
    resource_register_item(&backend->resource);
    resource_register_item(&protocol->resource);

}

