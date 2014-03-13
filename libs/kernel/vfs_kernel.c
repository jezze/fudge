#include <kernel.h>
#include "resource.h"
#include "vfs.h"
#include "task.h"
#include "container.h"
#include "kernel.h"

enum entry_type
{

    ENTRY_TYPE_NULL                     = 0x00000000,
    ENTRY_TYPE_ROOT                     = 0x01010000,
    ENTRY_TYPE_CONTAINERS               = 0x01020000,
    ENTRY_TYPE_TASKS                    = 0x01030000,
    ENTRY_TYPE_FILESYSTEMS              = 0x01040000,
    ENTRY_TYPE_FILESYSTEMBACKENDS       = 0x04050000,
    ENTRY_TYPE_FILESYSTEMPROTOCOLS      = 0x04060000,
    ENTRY_TYPE_BINARIES                 = 0x01070000

};

struct entry
{

    unsigned int (*read)(unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*walk)(unsigned int count, const char *path);

};

static unsigned int entry_root_read(unsigned int offset, unsigned int count, void *buffer);
static unsigned int entry_root_walk(unsigned int count, const char *path);
static unsigned int entry_containers_read(unsigned int offset, unsigned int count, void *buffer);
static unsigned int entry_containers_walk(unsigned int count, const char *path);
static unsigned int entry_tasks_read(unsigned int offset, unsigned int count, void *buffer);
static unsigned int entry_tasks_walk(unsigned int count, const char *path);
static unsigned int entry_filesystems_read(unsigned int offset, unsigned int count, void *buffer);
static unsigned int entry_filesystems_walk(unsigned int count, const char *path);
static unsigned int entry_filesystembackends_read(unsigned int offset, unsigned int count, void *buffer);
static unsigned int entry_filesystembackends_walk(unsigned int count, const char *path);
static unsigned int entry_filesystemprotocols_read(unsigned int offset, unsigned int count, void *buffer);
static unsigned int entry_filesystemprotocols_walk(unsigned int count, const char *path);
static unsigned int entry_binaries_read(unsigned int offset, unsigned int count, void *buffer);
static unsigned int entry_binaries_walk(unsigned int count, const char *path);

static struct entry entries[] = {
    {0, 0},
    {entry_root_read, entry_root_walk},
    {entry_containers_read, entry_containers_walk},
    {entry_tasks_read, entry_tasks_walk},
    {entry_filesystems_read, entry_filesystems_walk},
    {entry_filesystembackends_read, entry_filesystembackends_walk},
    {entry_filesystemprotocols_read, entry_filesystemprotocols_walk},
    {entry_binaries_read, entry_binaries_walk}
};

static unsigned int entry_root_read(unsigned int offset, unsigned int count, void *buffer)
{

    return memory_read(buffer, count, "../\ncontainers/\ntasks/\nfilesystems/\nbinaries/\n", 46, offset);

}

static unsigned int entry_root_walk(unsigned int count, const char *path)
{

    if (memory_match(path, "containers/", count))
        return ENTRY_TYPE_CONTAINERS >> 16;

    if (memory_match(path, "tasks/", count))
        return ENTRY_TYPE_TASKS >> 16;

    if (memory_match(path, "filesystems/", count))
        return ENTRY_TYPE_FILESYSTEMS >> 16;

    if (memory_match(path, "binaries/", count))
        return ENTRY_TYPE_BINARIES >> 16;

    return 0;

}

static unsigned int entry_containers_read(unsigned int offset, unsigned int count, void *buffer)
{

    return memory_read(buffer, count, "../\n", 4, offset);

}

static unsigned int entry_containers_walk(unsigned int count, const char *path)
{

    return 0;

}

static unsigned int entry_tasks_read(unsigned int offset, unsigned int count, void *buffer)
{

    return memory_read(buffer, count, "../\n", 4, offset);

}

static unsigned int entry_tasks_walk(unsigned int count, const char *path)
{

    return 0;

}

static unsigned int entry_filesystems_read(unsigned int offset, unsigned int count, void *buffer)
{

    return memory_read(buffer, count, "../\nbackends/\nprotocols/\n", 25, offset);

}

static unsigned int entry_filesystems_walk(unsigned int count, const char *path)
{

    if (memory_match(path, "backends/", count))
        return ENTRY_TYPE_FILESYSTEMBACKENDS >> 16;

    if (memory_match(path, "protocols/", count))
        return ENTRY_TYPE_FILESYSTEMPROTOCOLS >> 16;

    return 0;

}

static unsigned int entry_filesystembackends_read(unsigned int offset, unsigned int count, void *buffer)
{

    return memory_read(buffer, count, "../\n", 4, offset);

}

static unsigned int entry_filesystembackends_walk(unsigned int count, const char *path)
{

    return 0;

}

static unsigned int entry_filesystemprotocols_read(unsigned int offset, unsigned int count, void *buffer)
{

    return memory_read(buffer, count, "../\n", 4, offset);

}

static unsigned int entry_filesystemprotocols_walk(unsigned int count, const char *path)
{

    return 0;

}

static unsigned int entry_binaries_read(unsigned int offset, unsigned int count, void *buffer)
{

    return memory_read(buffer, count, "../\n", 4, offset);

}

static unsigned int entry_binaries_walk(unsigned int count, const char *path)
{

    return 0;

}

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

    return id >> 8;

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

    return entries[id & 0xFF].read(offset, count, buffer);

}

static unsigned int write(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int walk(struct vfs_backend *backend, unsigned int id, unsigned int count, const char *path)
{

    if (!count)
        return id;

    return entries[id & 0xFF].walk(count, path);

}

void vfs_setup_kernel(struct vfs_backend *backend, struct vfs_protocol *protocol)
{

    vfs_init_backend(backend, backend_read, backend_write);
    vfs_init_protocol(protocol, match, root, open, close, read, write, parent, walk, get_physical);
    resource_register_item(&backend->resource);
    resource_register_item(&protocol->resource);

}

