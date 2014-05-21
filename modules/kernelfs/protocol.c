#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include "protocol.h"

static unsigned int root(struct vfs_backend *backend)
{

    return 0;

}

static unsigned int parent(struct vfs_backend *backend, unsigned int id)
{

    return 0;

}

static unsigned int match(struct vfs_backend *backend)
{

    unsigned char buffer[8];

    backend->read(backend, 0, 8, buffer);

    return memory_match(buffer, "KERNELFS", 8);

}

static unsigned int open(struct vfs_backend *backend, unsigned int id)
{

    return 0;

}

static unsigned int close(struct vfs_backend *backend, unsigned int id)
{

    return 0;

}

static unsigned int read(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int child(struct vfs_backend *backend, unsigned int id, unsigned int count, const char *path)
{

    return 0;

}

static unsigned int write(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

void kernelfs_init_protocol(struct vfs_protocol *protocol)
{

    memory_clear(protocol, sizeof (struct vfs_protocol));
    vfs_init_protocol(protocol, match, root, open, close, read, write, parent, child, 0);

}

