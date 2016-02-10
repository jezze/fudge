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

static unsigned int protocol_open(struct service_backend *backend, struct list_item *link, struct service_state *state)
{

    struct system_node *node = (struct system_node *)state->id;

    state->offset = 0;
    node->refcount++;

    switch (node->type)
    {

    case SYSTEM_NODETYPE_MAILBOX:
        list_add(&node->links, link);

        break;

    }

    return state->id;

}

static unsigned int protocol_close(struct service_backend *backend, struct list_item *link, struct service_state *state)
{

    struct system_node *node = (struct system_node *)state->id;

    state->offset = 0;
    node->refcount--;

    switch (node->type)
    {

    case SYSTEM_NODETYPE_MAILBOX:
        list_remove(&node->links, link);

        break;

    }

    return state->id;

}

static unsigned int readmailbox(struct task *task, unsigned int count, void *buffer)
{

    count = buffer_rcfifo(&task->mailbox.buffer, count, buffer);

    if (!count)
        task_setstatus(task, TASK_STATUS_BLOCKED);

    return count;

}

static unsigned int protocol_read(struct service_backend *backend, struct list_item *link, struct service_state *state, unsigned int count, void *buffer)
{

    struct system_node *node = (struct system_node *)state->id;

    switch (node->type)
    {

    case SYSTEM_NODETYPE_MAILBOX:
        return readmailbox(link->data, count, buffer);

    }

    count = (node->read) ? node->read(node, link, state->offset, count, buffer) : 0;
    state->offset += count;

    return count;

}

static unsigned int protocol_write(struct service_backend *backend, struct list_item *link, struct service_state *state, unsigned int count, void *buffer)
{

    struct system_node *node = (struct system_node *)state->id;

    count = (node->write) ? node->write(node, link, state->offset, count, buffer) : 0;
    state->offset += count;

    return count;

}

static unsigned int protocol_seek(struct service_backend *backend, struct service_state *state, unsigned int offset)
{

    return state->offset = offset;

}

static unsigned int protocol_scan(struct service_backend *backend, struct service_state *state, unsigned int index)
{

    struct system_node *node = (struct system_node *)state->id;

    return (node->scan) ? node->scan(node, index) : 0;

}

void system_initprotocol(struct service_protocol *protocol)
{

    service_initprotocol(protocol, protocol_match, protocol_root, protocol_parent, protocol_child, protocol_create, protocol_destroy, protocol_open, protocol_close, protocol_read, protocol_write, protocol_seek, protocol_scan, 0);

}

