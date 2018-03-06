#include <fudge.h>
#include <kernel.h>
#include "system.h"

static struct system_node root;
static struct system_header header;
static struct service_backend backend;

static unsigned int backend_read(void *buffer, unsigned int count, unsigned int offset)
{

    return 0;

}

static unsigned int backend_write(void *buffer, unsigned int count, unsigned int offset)
{

    return 0;

}

static unsigned int backend_map(struct service_state *state, unsigned int offset, unsigned int count)
{

    if (!offset && count == sizeof (struct system_header))
        return (unsigned int)&header;
    else
        return offset;

}

void system_registernode(struct system_node *node)
{

    system_addchild(&root, node);

}

void system_unregisternode(struct system_node *node)
{

    system_removechild(&root, node);

}

void system_initbackend(void)
{

    service_initbackend(&backend, 2000, backend_read, backend_write, backend_map);
    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "FUDGE_ROOT");
    memory_write(header.id, 12, "FUDGE_SYSTEM", 12, 0);

    root.parent = &root;
    header.root = (unsigned int )&root;

    resource_register(&backend.resource);

}

