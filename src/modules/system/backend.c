#include <fudge.h>
#include <kernel.h>
#include "system.h"

static struct system_node root;
static struct system_header header;

static unsigned int backend_read(unsigned int offset, unsigned int count, void *buffer)
{

    return memory_read(buffer, count, &header, sizeof (struct system_header), offset);

}

static unsigned int backend_write(unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

void system_registernode(struct system_node *node)
{

    system_addchild(&root, node);

}

void system_unregisternode(struct system_node *node)
{

    system_removechild(&root, node);

}

void system_initbackend(struct vfs_backend *backend)
{

    vfs_initbackend(backend, 2000, backend_read, backend_write, 0);
    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "FUDGE_ROOT");
    memory_write(header.id, 12, "FUDGE_SYSTEM", 12, 0);

    root.parent = &root;
    header.root = (unsigned int )&root;

}

