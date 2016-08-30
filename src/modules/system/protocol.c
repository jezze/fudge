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

static unsigned int protocol_parent(struct service_backend *backend, struct service_state *state)
{

    struct system_node *node = (struct system_node *)state->id;

    state->id = (unsigned int)node->parent;

    return state->id != 0;

}

static unsigned int protocol_child(struct service_backend *backend, struct service_state *state, char *path, unsigned int length)
{

    struct system_node *node = (struct system_node *)state->id;

    state->id = (node->child) ? node->child(node, path, length) : (length ? 0 : state->id);

    return state->id != 0;

}

static unsigned int protocol_create(struct service_backend *backend, struct service_state *state, char *name, unsigned int length)
{

    return 0;

}

static unsigned int protocol_destroy(struct service_backend *backend, struct service_state *state, char *name, unsigned int length)
{

    return 0;

}

static unsigned int protocol_step(struct service_backend *backend, unsigned int id, unsigned int current)
{

    if (!current)
    {

        struct system_node *node = (struct system_node *)id;

        return (node->children.head) ? (unsigned int)node->children.head->data : 0;

    }

    else
    {

        struct system_node *node = (struct system_node *)current;

        return (node->item.next) ? (unsigned int)node->item.next->data : 0;

    }

}

static unsigned int protocol_open(struct service_backend *backend, struct service_state *state)
{

    struct system_node *node = (struct system_node *)state->id;

    return (node->open) ? node->open(node, state) : state->id;

}

static unsigned int protocol_close(struct service_backend *backend, struct service_state *state)
{

    struct system_node *node = (struct system_node *)state->id;

    return (node->close) ? node->close(node, state) : state->id;

}

static unsigned int protocol_read(struct service_backend *backend, struct service_state *state, void *buffer, unsigned int count)
{

    struct system_node *node = (struct system_node *)state->id;

    return (node->read) ? node->read(node, state, buffer, count) : 0;
        
}

static unsigned int protocol_write(struct service_backend *backend, struct service_state *state, void *buffer, unsigned int count)
{

    struct system_node *node = (struct system_node *)state->id;

    return (node->write) ? node->write(node, state, buffer, count) : 0;

}

static unsigned int protocol_seek(struct service_backend *backend, unsigned int offset)
{

    return offset;

}

static unsigned long protocol_map(struct service_backend *backend, unsigned int id, struct binary_node *node)
{

    return 0;

}

void system_initprotocol(struct service_protocol *protocol)
{

    service_initprotocol(protocol, protocol_match, protocol_root, protocol_parent, protocol_child, protocol_create, protocol_destroy, protocol_step, protocol_open, protocol_close, protocol_read, protocol_write, protocol_seek, protocol_map);

}

