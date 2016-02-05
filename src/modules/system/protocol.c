#include <fudge.h>
#include <kernel.h>
#include "system.h"

static unsigned int protocol_match(struct service_backend *backend)
{

    struct system_header header;

    if (backend->read(0, sizeof (struct system_header), &header) < sizeof (struct system_header))
        return 0;

    return memory_match(header.id, "FUDGE_SYSTEM", 12);

}

static unsigned int protocol_root(struct service_backend *backend)
{

    struct system_header header;

    backend->read(0, sizeof (struct system_header), &header);

    return header.root;

}

static unsigned int protocol_parent(struct service_backend *backend, unsigned int id)
{

    struct system_node *node = (struct system_node *)id;

    return (unsigned int)node->parent;

}

static unsigned int protocol_child(struct service_backend *backend, unsigned int id, unsigned int count, char *path)
{

    struct system_node *node = (struct system_node *)id;

    return (node->child) ? node->child(node, count, path) : (count ? 0 : id);

}

static unsigned int protocol_create(struct service_backend *backend, unsigned int id, unsigned int count, char *name)
{

    return 0;

}

static unsigned int protocol_destroy(struct service_backend *backend, unsigned int id, unsigned int count, char *name)
{

    return 0;

}

static unsigned int protocol_open(struct service_backend *backend, struct task *task, unsigned int descriptor, unsigned int id)
{

    struct system_node *node = (struct system_node *)id;

    node->refcount++;

    return (node->open) ? node->open(node, task, descriptor) : id;

}

static unsigned int protocol_close(struct service_backend *backend, struct task *task, unsigned int descriptor, unsigned int id)
{

    struct system_node *node = (struct system_node *)id;

    node->refcount--;

    return (node->close) ? node->close(node, task, descriptor) : id;

}

static unsigned int protocol_read(struct service_backend *backend, struct task *task, unsigned int descriptor, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct system_node *node = (struct system_node *)id;

    return (node->read) ? node->read(node, task, descriptor, offset, count, buffer) : 0;

}

static unsigned int protocol_write(struct service_backend *backend, struct task *task, unsigned int descriptor, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct system_node *node = (struct system_node *)id;

    return (node->write) ? node->write(node, task, descriptor, offset, count, buffer) : 0;

}

static unsigned int protocol_seek(struct service_backend *backend, unsigned int id, unsigned int offset)
{

    return offset;

}

static unsigned int protocol_scan(struct service_backend *backend, unsigned int id, unsigned int index)
{

    struct system_node *node = (struct system_node *)id;

    return (node->scan) ? node->scan(node, index) : 0;

}

void system_initprotocol(struct service_protocol *protocol)
{

    service_initprotocol(protocol, protocol_match, protocol_root, protocol_parent, protocol_child, protocol_create, protocol_destroy, protocol_open, protocol_close, protocol_read, protocol_write, protocol_seek, protocol_scan, 0);

}

