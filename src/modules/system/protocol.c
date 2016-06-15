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

static unsigned int protocol_parent(struct service_state *state)
{

    struct system_node *node = (struct system_node *)state->id;

    return (unsigned int)node->parent;

}

static unsigned int protocol_child(struct service_state *state, unsigned int count, char *path)
{

    struct system_node *node = (struct system_node *)state->id;

    return (node->child) ? node->child(node, count, path) : (count ? 0 : state->id);

}

static unsigned int protocol_create(struct service_state *state, unsigned int count, char *name)
{

    return 0;

}

static unsigned int protocol_destroy(struct service_state *state, unsigned int count, char *name)
{

    return 0;

}

static unsigned int protocol_open(struct service_state *state)
{

    struct system_node *node = (struct system_node *)state->id;

    state->offset = 0;
    node->refcount++;

    switch (node->type)
    {

    case SYSTEM_NODETYPE_GROUP:
    case SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI:
        state->offset = (node->children.head) ? (unsigned int)node->children.head->data : 0;

        break;

    case SYSTEM_NODETYPE_MAILBOX:
        list_add(&node->links, state->link);

        break;

    }

    return state->id;

}

static unsigned int protocol_close(struct service_state *state)
{

    struct system_node *node = (struct system_node *)state->id;

    state->offset = 0;
    node->refcount--;

    switch (node->type)
    {

    case SYSTEM_NODETYPE_MAILBOX:
        list_remove(&node->links, state->link);

        break;

    }

    return state->id;

}

static unsigned int readgroup(struct system_node *self, struct service_state *state, unsigned int count, void *buffer)
{

    struct record *record = buffer;
    struct system_node *node = (struct system_node *)state->offset;

    if (!state->offset)
        return 0;

    record->id = state->offset;
    record->size = 0;
    record->length = memory_read(record->name, RECORD_NAMESIZE, node->name, ascii_length(node->name), 0);

    if (node->type & SYSTEM_NODETYPE_MULTI)
    {

        char *index = ":0";

        index[1] = '0' + node->index;

        record->length += memory_write(record->name, RECORD_NAMESIZE, index, 2, record->length);

    }

    if (node->type & SYSTEM_NODETYPE_GROUP)
        record->length += memory_write(record->name, RECORD_NAMESIZE, "/", 1, record->length);

    state->offset = (node->item.next) ? (unsigned int)node->item.next->data : 0;

    return sizeof (struct record);

}

static unsigned int readmailbox(struct system_node *self, struct service_state *state, unsigned int count, void *buffer)
{

    struct task *task = state->link->data;

    count = buffer_rcfifo(&task->mailbox.buffer, count, buffer);

    if (!count)
        task_setstatus(task, TASK_STATUS_BLOCKED);

    state->offset += count;

    return count;

}

static unsigned int protocol_read(struct service_state *state, unsigned int count, void *buffer)
{

    struct system_node *node = (struct system_node *)state->id;

    switch (node->type)
    {

    case SYSTEM_NODETYPE_GROUP:
    case SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI:
        return readgroup(node, state, count, buffer);

    case SYSTEM_NODETYPE_MAILBOX:
        return readmailbox(node, state, count, buffer);

    }

    count = (node->read) ? node->read(node, state, count, buffer) : 0;
    state->offset += count;

    return count;

}

static unsigned int protocol_write(struct service_state *state, unsigned int count, void *buffer)
{

    struct system_node *node = (struct system_node *)state->id;

    count = (node->write) ? node->write(node, state, count, buffer) : 0;
    state->offset += count;

    return count;

}

static unsigned int protocol_seek(struct service_state *state, unsigned int offset)
{

    return state->offset = offset;

}

static unsigned long protocol_map(struct service_state *state, struct binary_node *node)
{

    return 0;

}

void system_initprotocol(struct service_protocol *protocol)
{

    service_initprotocol(protocol, protocol_match, protocol_root, protocol_parent, protocol_child, protocol_create, protocol_destroy, protocol_open, protocol_close, protocol_read, protocol_write, protocol_seek, protocol_map);

}

