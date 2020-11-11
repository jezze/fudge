#include <fudge.h>
#include <kernel.h>
#include "system.h"

static struct service_protocol protocol;

static unsigned int protocol_match(struct service_backend *backend)
{

    struct system_header *header = (struct system_header *)backend->map(0, sizeof (struct system_header));

    return (header) ? memory_match(header->id, "FUDGE_SYSTEM", 12) : 0;

}

static unsigned int protocol_root(struct service_backend *backend)
{

    struct system_header *header = (struct system_header *)backend->map(0, sizeof (struct system_header));

    return (header) ? header->root : 0;

}

static unsigned int protocol_parent(struct service_backend *backend, unsigned int id)
{

    struct system_node *node = (struct system_node *)backend->map(id, sizeof (struct system_node));

    return (unsigned int)node->parent;

}

static unsigned int protocol_child(struct service_backend *backend, unsigned int id, char *path, unsigned int length)
{

    struct system_node *node = (struct system_node *)backend->map(id, sizeof (struct system_node));
    struct list_item *current;
    struct system_node *n = node;

    spinlock_acquire(&node->children.spinlock);

    for (current = node->children.head; current; current = current->next)
    {

        struct system_node *n2 = current->data;
        unsigned int length0 = ascii_length(n2->name);

        if (n2->type == SYSTEM_NODETYPE_MULTIGROUP)
        {

            unsigned int colon = memory_findbyte(path, length, ':');
            unsigned int val;

            if (length0 != colon)
                continue;

            if (!memory_match(n2->name, path, colon))
                continue;

            val = ascii_rvalue(path + colon + 1, length - colon - 1, 10);

            if (val != n2->index)
                continue;

        }

        else
        {

            if (length0 != length)
                continue;

            if (!memory_match(n2->name, path, length))
                continue;

        }

        n = n2;

        break;

    }

    spinlock_release(&node->children.spinlock);

    return (unsigned int)n;

}

static unsigned int protocol_create(struct service_backend *backend, unsigned int id, char *name, unsigned int length)
{

    return 0;

}

static unsigned int protocol_destroy(struct service_backend *backend, unsigned int id, char *name, unsigned int length)
{

    return 0;

}

static unsigned int protocol_step(struct service_backend *backend, unsigned int id, unsigned int current)
{

    struct system_node *node = (struct system_node *)backend->map(current, sizeof (struct system_node));

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

    struct system_node *node = (struct system_node *)backend->map(id, sizeof (struct system_node));

    return (unsigned int)node->operations.open(node, state);

}

static unsigned int protocol_close(struct service_backend *backend, struct service_state *state, unsigned int id)
{

    struct system_node *node = (struct system_node *)backend->map(id, sizeof (struct system_node));

    return (unsigned int)node->operations.close(node, state);

}

static unsigned int protocol_read(struct service_backend *backend, struct service_state *state, unsigned int id, unsigned int current, void *buffer, unsigned int count, unsigned int offset)
{

    struct system_node *node = (struct system_node *)backend->map(id, sizeof (struct system_node));
    struct system_node *currentnode = (struct system_node *)backend->map(current, sizeof (struct system_node));

    return node->operations.read(node, currentnode, state, buffer, count, offset);

}

static unsigned int protocol_write(struct service_backend *backend, struct service_state *state, unsigned int id, unsigned int current, void *buffer, unsigned int count, unsigned int offset)
{

    struct system_node *node = (struct system_node *)backend->map(id, sizeof (struct system_node));
    struct system_node *currentnode = (struct system_node *)backend->map(current, sizeof (struct system_node));

    return node->operations.write(node, currentnode, state, buffer, count, offset);

}

static unsigned int protocol_seek(struct service_backend *backend, unsigned int id, unsigned int offset)
{

    struct system_node *node = (struct system_node *)backend->map(id, sizeof (struct system_node));

    return node->operations.seek(node, offset);

}

static unsigned int protocol_map(struct service_backend *backend, unsigned int id)
{

    return backend->map(id, sizeof (struct system_node));

}

void system_initprotocol(void)
{

    service_initprotocol(&protocol, 2000, protocol_match, protocol_root, protocol_parent, protocol_child, protocol_create, protocol_destroy, protocol_step, protocol_open, protocol_close, protocol_read, protocol_write, protocol_seek, protocol_map);
    resource_register(&protocol.resource);

}

