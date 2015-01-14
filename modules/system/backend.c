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

static void setupheader(struct system_header *header, struct system_node *root)
{

    memory_write(header->id, 12, "FUDGE_SYSTEM", 12, 0);

    header->root = root;

}

void system_initbackend(struct system_backend *backend)
{

    vfs_initbackend(&backend->base, 2000, backend_read, backend_write);
    system_initnode(&backend->root, SYSTEM_NODETYPE_GROUP, "FUDGE_ROOT");
    setupheader(&backend->header, &backend->root);

    backend->root.parent = &backend->root;

}

