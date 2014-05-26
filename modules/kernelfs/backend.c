#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include "backend.h"

static unsigned int read(struct vfs_backend *self, unsigned int offset, unsigned int count, void *buffer)
{

    return memory_read(buffer, count, "KERNELFS", 8, offset);

}

static unsigned int write(struct vfs_backend *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

void kernelfs_init_backend(struct vfs_backend *backend)
{

    memory_clear(backend, sizeof (struct vfs_backend));
    vfs_init_backend(backend, 3000, read, write);

}

