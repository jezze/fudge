#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include "system.h"
#include "protocol.h"

static unsigned int root(struct vfs_backend *backend)
{

    struct system_header header;

    backend->read(backend, 0, sizeof (struct system_header), &header);

    return (unsigned int)header.root;

}

static unsigned int parent(struct vfs_backend *backend, unsigned int id)
{

    struct system_node *node = (struct system_node *)(unsigned int)id;

    return (unsigned int)node->parent;

}

static unsigned int match(struct vfs_backend *backend)
{

    struct system_header header;
    
    if (backend->read(backend, 0, sizeof (struct system_header), &header) < sizeof (struct system_header))
        return 0;

    return memory_match(header.id, "FUDGE_SYSTEM", 12);

}

static unsigned int open(struct vfs_backend *backend, unsigned int id)
{

    struct system_node *node = (struct system_node *)(unsigned int)id;

    return node->open(node);

}

static unsigned int close(struct vfs_backend *backend, unsigned int id)
{

    struct system_node *node = (struct system_node *)(unsigned int)id;

    return node->close(node);

}

static unsigned int read(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct system_node *node = (struct system_node *)(unsigned int)id;

    return node->read(node, offset, count, buffer);

}

static unsigned int child(struct vfs_backend *backend, unsigned int id, unsigned int count, const char *path)
{

    struct system_node *node = (struct system_node *)(unsigned int)id;

    return node->child(node, count, path);

}

static unsigned int write(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct system_node *node = (struct system_node *)(unsigned int)id;

    return node->write(node, offset, count, buffer);

}

void system_init_protocol(struct vfs_protocol *protocol)
{

    memory_clear(protocol, sizeof (struct vfs_protocol));
    vfs_init_protocol(protocol, match, root, open, close, read, write, parent, child, 0);

}

