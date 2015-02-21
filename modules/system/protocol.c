#include <fudge.h>
#include <kernel.h>
#include "system.h"
#include "protocol.h"

static struct system_node *getnode(unsigned int id)
{

    return (struct system_node *)id;

}

static unsigned int protocol_match(struct vfs_backend *backend)
{

    struct system_header header;
    
    if (backend->read(backend, 0, sizeof (struct system_header), &header) < sizeof (struct system_header))
        return 0;

    return memory_match(header.id, "FUDGE_SYSTEM", 12);

}

static unsigned int protocol_root(struct vfs_backend *backend)
{

    struct system_header header;

    backend->read(backend, 0, sizeof (struct system_header), &header);

    return header.root;

}

static unsigned int protocol_parent(struct vfs_backend *backend, unsigned int id)
{

    struct system_node *node = getnode(id);

    return (unsigned int)node->parent;

}

static unsigned int protocol_child(struct vfs_backend *backend, unsigned int id, unsigned int count, const char *path)
{

    struct system_node *node = getnode(id);

    return node->child(node, count, path);

}

static unsigned int protocol_create(struct vfs_backend *backend, unsigned int id, unsigned int count, const char *name)
{

    return 0;

}

static unsigned int protocol_destroy(struct vfs_backend *backend, unsigned int id, unsigned int count, const char *name)
{

    return 0;

}

static unsigned int protocol_open(struct vfs_backend *backend, unsigned int id)
{

    struct system_node *node = getnode(id);

    return node->open(node);

}

static unsigned int protocol_close(struct vfs_backend *backend, unsigned int id)
{

    struct system_node *node = getnode(id);

    return node->close(node);

}

static unsigned int protocol_read(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct system_node *node = getnode(id);

    return node->read(node, offset, count, buffer);

}

static unsigned int protocol_write(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct system_node *node = getnode(id);

    return node->write(node, offset, count, buffer);

}

void system_initprotocol(struct vfs_protocol *protocol)
{

    vfs_initprotocol(protocol, protocol_match, protocol_root, protocol_parent, protocol_child, protocol_create, protocol_destroy, protocol_open, protocol_close, protocol_read, protocol_write, 0);

}

