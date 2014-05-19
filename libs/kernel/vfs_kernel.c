#include <kernel.h>
#include "resource.h"
#include "vfs.h"
#include "task.h"
#include "container.h"
#include "kernel.h"

static unsigned int backend_read(struct vfs_backend *self, unsigned int offset, unsigned int count, void *buffer)
{

    return memory_read(buffer, count, "FUDGE_INTERNAL", 14, offset);

}

static unsigned int backend_write(struct vfs_backend *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

void vfs_setup_kernel(struct vfs_backend *backend, struct vfs_protocol *protocol)
{

    vfs_init_backend(backend, backend_read, backend_write);
    resource_register_item(&backend->resource);

}

