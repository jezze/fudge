#include <fudge.h>
#include <kernel.h>
#include "system.h"

static struct service_protocol protocol;

static unsigned int protocol_match(struct service_backend *backend, struct service_state *state)
{

    struct system_header *header = (struct system_header *)backend->map(state, 0, sizeof (struct system_header));

    return (header) ? memory_match(header->id, "FUDGE_SYSTEM", 12) : 0;

}

static unsigned int protocol_root(struct service_backend *backend, struct service_state *state)
{

    struct system_header *header = (struct system_header *)backend->map(state, 0, sizeof (struct system_header));

    return (header) ? header->root : 0;

}

static unsigned int protocol_parent(struct service_backend *backend, struct service_state *state, unsigned int id)
{

    struct system_node *node = (struct system_node *)backend->map(state, id, sizeof (struct system_node));

    return (unsigned int)node->parent;

}

static unsigned int protocol_child(struct service_backend *backend, struct service_state *state, unsigned int id, char *path, unsigned int length)
{

    struct system_node *node = (struct system_node *)backend->map(state, id, sizeof (struct system_node));

    return (unsigned int)node->operations.child(node, state, path, length);

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

    struct system_node *node = (struct system_node *)backend->map(state, current, sizeof (struct system_node));

    if (id == current)
    {

        spinlock_acquire(&node->children.spinlock);

        id = (node->children.head) ? (unsigned int)node->children.head->data : 0;

        spinlock_release(&node->children.spinlock);

        return id;

    }

    else
    {

        return (node->item.next) ? (unsigned int)node->item.next->data : 0;

    }

}

static unsigned int protocol_open(struct service_backend *backend, struct service_state *state, unsigned int id)
{

    struct system_node *node = (struct system_node *)backend->map(state, id, sizeof (struct system_node));

    return (unsigned int)node->operations.open(node, state);

}

static unsigned int protocol_close(struct service_backend *backend, struct service_state *state, unsigned int id)
{

    struct system_node *node = (struct system_node *)backend->map(state, id, sizeof (struct system_node));

    return (unsigned int)node->operations.close(node, state);

}

static unsigned int protocol_read(struct service_backend *backend, struct service_state *state, unsigned int id, unsigned int current, void *buffer, unsigned int count, unsigned int offset)
{

    struct system_node *node = (struct system_node *)backend->map(state, id, sizeof (struct system_node));
    struct system_node *currentnode = (struct system_node *)backend->map(state, current, sizeof (struct system_node));

    return node->operations.read(node, currentnode, state, buffer, count, offset);

}

static unsigned int protocol_write(struct service_backend *backend, struct service_state *state, unsigned int id, unsigned int current, void *buffer, unsigned int count, unsigned int offset)
{

    struct system_node *node = (struct system_node *)backend->map(state, id, sizeof (struct system_node));
    struct system_node *currentnode = (struct system_node *)backend->map(state, current, sizeof (struct system_node));

    return node->operations.write(node, currentnode, state, buffer, count, offset);

}

static unsigned int protocol_seek(struct service_backend *backend, struct service_state *state, unsigned int id, unsigned int offset)
{

    struct system_node *node = (struct system_node *)backend->map(state, id, sizeof (struct system_node));

    return node->operations.seek(node, state, offset);

}

static unsigned int protocol_map(struct service_backend *backend, struct service_state *state, unsigned int id)
{

    return backend->map(state, id, sizeof (struct system_node));

}

void system_initprotocol(void)
{

    service_initprotocol(&protocol, 2000, protocol_match, protocol_root, protocol_parent, protocol_child, protocol_create, protocol_destroy, protocol_step, protocol_open, protocol_close, protocol_read, protocol_write, protocol_seek, protocol_map);
    resource_register(&protocol.resource);

}

