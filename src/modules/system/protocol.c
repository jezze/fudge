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

static unsigned int protocol_parent(struct service_descriptor *descriptor)
{

    struct system_node *node = (struct system_node *)descriptor->id;

    descriptor->id = (unsigned int)node->parent;

    return descriptor->id != 0;

}

static unsigned int protocol_child(struct service_descriptor *descriptor, char *path, unsigned int length)
{

    struct system_node *node = (struct system_node *)descriptor->id;

    descriptor->id = (node->child) ? node->child(node, descriptor, path, length) : (length ? 0 : descriptor->id);

    return descriptor->id != 0;

}

static unsigned int protocol_create(struct service_descriptor *descriptor, char *name, unsigned int length)
{

    return 0;

}

static unsigned int protocol_destroy(struct service_descriptor *descriptor, char *name, unsigned int length)
{

    return 0;

}

static unsigned int protocol_step(struct service_descriptor *descriptor)
{

    if (descriptor->current)
    {

        struct system_node *node = (struct system_node *)descriptor->current;

        return (node->item.next) ? (unsigned int)node->item.next->data : 0;

    }

    else
    {

        struct system_node *node = (struct system_node *)descriptor->id;

        return (node->children.head) ? (unsigned int)node->children.head->data : 0;

    }

}

static unsigned int protocol_open(struct service_descriptor *descriptor)
{

    struct system_node *node = (struct system_node *)descriptor->id;

    return (node->open) ? node->open(node, descriptor) : descriptor->id;

}

static unsigned int protocol_close(struct service_descriptor *descriptor)
{

    struct system_node *node = (struct system_node *)descriptor->id;

    return (node->close) ? node->close(node, descriptor) : descriptor->id;

}

static unsigned int protocol_read(struct service_descriptor *descriptor, void *buffer, unsigned int count)
{

    struct system_node *node = (struct system_node *)descriptor->id;

    return (node->read) ? node->read(node, descriptor, buffer, count) : 0;
        
}

static unsigned int protocol_write(struct service_descriptor *descriptor, void *buffer, unsigned int count)
{

    struct system_node *node = (struct system_node *)descriptor->id;

    return (node->write) ? node->write(node, descriptor, buffer, count) : 0;

}

static unsigned int protocol_seek(struct service_descriptor *descriptor, unsigned int offset)
{

    return offset;

}

static unsigned int protocol_map(struct service_descriptor *descriptor)
{

    return 0;

}

void system_initprotocol(struct service_protocol *protocol)
{

    service_initprotocol(protocol, protocol_match, protocol_root, protocol_parent, protocol_child, protocol_create, protocol_destroy, protocol_step, protocol_open, protocol_close, protocol_read, protocol_write, protocol_seek, protocol_map);

}

