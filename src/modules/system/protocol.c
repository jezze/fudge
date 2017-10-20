#include <fudge.h>
#include <kernel.h>
#include "system.h"

static unsigned int protocol_match(struct service_backend *backend)
{

    struct system_header header;

    if (backend->read(&header, sizeof (struct system_header), 0) < sizeof (struct system_header))
        return 0;

    return memory_match(header.id, "FUDGE_SYSTEM", 12);

}

static unsigned int protocol_root(struct service_backend *backend)
{

    struct system_header header;

    backend->read(&header, sizeof (struct system_header), 0);

    return header.root;

}

static unsigned int protocol_parent(struct service_backend *backend, struct service_state *state, unsigned int id)
{

    struct system_node *node = (struct system_node *)id;

    return (unsigned int)node->parent;

}

static unsigned int protocol_child(struct service_backend *backend, struct service_state *state, unsigned int id, char *path, unsigned int length)
{

    struct system_node *node = (struct system_node *)id;

    return (node->child) ? node->child(node, state, path, length) : id;

}

static unsigned int protocol_create(struct service_backend *backend, struct service_state *state, unsigned int id, char *name, unsigned int length)
{

    return 0;

}

static unsigned int protocol_destroy(struct service_backend *backend, struct service_state *state, unsigned int id, char *name, unsigned int length)
{

    return 0;

}

static unsigned int protocol_step(struct service_backend *backend, struct service_state *state, unsigned int id, unsigned int current)
{

    struct system_node *node = (struct system_node *)current;

    if (id == current)
    {

        spinlock_acquire(&node->childlock);

        id = (node->children.head) ? (unsigned int)node->children.head->data : 0;

        spinlock_release(&node->childlock);

        return id;

    }

    else
    {

        return (node->item.next) ? (unsigned int)node->item.next->data : 0;

    }

}

static unsigned int protocol_open(struct service_backend *backend, struct service_state *state, unsigned int id)
{

    struct system_node *node = (struct system_node *)id;

    return (node->open) ? node->open(node, state) : id;

}

static unsigned int protocol_close(struct service_backend *backend, struct service_state *state, unsigned int id)
{

    struct system_node *node = (struct system_node *)id;

    return (node->close) ? node->close(node, state) : id;

}

static unsigned int protocol_read(struct service_backend *backend, struct service_state *state, unsigned int id, unsigned int current, void *buffer, unsigned int count, unsigned int offset)
{

    struct system_node *node = (struct system_node *)id;
    struct system_node *currentnode = (struct system_node *)current;

    return (node->read) ? node->read(node, currentnode, state, buffer, count, offset) : 0;

}

static unsigned int protocol_write(struct service_backend *backend, struct service_state *state, unsigned int id, unsigned int current, void *buffer, unsigned int count, unsigned int offset)
{

    struct system_node *node = (struct system_node *)id;
    struct system_node *currentnode = (struct system_node *)current;

    return (node->write) ? node->write(node, currentnode, state, buffer, count, offset) : 0;

}

static unsigned int protocol_seek(struct service_backend *backend, struct service_state *state, unsigned int id, unsigned int offset)
{

    struct system_node *node = (struct system_node *)id;

    return (node->seek) ? node->seek(node, state, offset) : 0;

}

static unsigned int protocol_map(struct service_backend *backend, struct service_state *state, unsigned int id)
{

    return 0;

}

void system_initprotocol(struct service_protocol *protocol)
{

    service_initprotocol(protocol, 2000, protocol_match, protocol_root, protocol_parent, protocol_child, protocol_create, protocol_destroy, protocol_step, protocol_open, protocol_close, protocol_read, protocol_write, protocol_seek, protocol_map);

}

