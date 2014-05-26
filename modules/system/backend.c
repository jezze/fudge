#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include "system.h"
#include "backend.h"

static unsigned int backend_read(struct vfs_backend *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct system_backend *backend = (struct system_backend *)self;

    return memory_read(buffer, count, &backend->header, sizeof (struct system_header), offset);

}

static unsigned int backend_write(struct vfs_backend *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

void system_init_header(struct system_header *header, struct system_group *root)
{

    memory_clear(header, sizeof (struct system_header));
    memory_copy(header->id, "FUDGE_SYSTEM", 12);

    header->root = root;

}

void system_init_backend(struct system_backend *backend)
{

    memory_clear(backend, sizeof (struct system_backend));
    vfs_init_backend(&backend->base, 2000, backend_read, backend_write);
    system_init_group(&backend->root, "FUDGE_ROOT");
    system_init_header(&backend->header, &backend->root);

    backend->root.node.parent = &backend->root.node;

}

