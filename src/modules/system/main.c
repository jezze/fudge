#include <fudge.h>
#include <kernel.h>
#include "system.h"

static struct service_backend backend;
static struct service_protocol protocol;

unsigned int system_opengroup(struct system_node *self, struct service_state *state)
{

    state->offset = (self->children.head) ? (unsigned int)self->children.head->data : 0;

    return state->id;

}

unsigned int system_openmailbox(struct system_node *self, struct service_state *state)
{

    list_add(&self->links, &state->link);

    return state->id;

}

unsigned int system_closegroup(struct system_node *self, struct service_state *state)
{

    return state->id;

}

unsigned int system_closemailbox(struct system_node *self, struct service_state *state)
{

    list_remove(&self->links, &state->link);

    return state->id;

}

unsigned int system_childgroup(struct system_node *node, char *path, unsigned int length)
{

    struct list_item *current;

    if (!length)
        return (unsigned int)node;

    for (current = node->children.head; current; current = current->next)
    {

        struct system_node *node = current->data;
        unsigned int length0 = ascii_length(node->name);

        if (!memory_match(node->name, path, length0))
            continue;

        if (node->type & SYSTEM_NODETYPE_MULTI)
        {

            unsigned int val;

            if (path[length0] != ':')
                continue;

            val = path[length0 + 1] - '0';

            if (val != node->index)
                continue;

            length0 += 2;

        }

        if (node->type & SYSTEM_NODETYPE_GROUP)
            length0 += 1;

        length -= length0;
        path += length0;

        return (node->child) ? node->child(node, path, length) : (length ? 0 : (unsigned int)node);

    }

    return 0;

}

unsigned int system_readmailbox(struct system_node *node, struct service_state *state, void *buffer, unsigned int count)
{

    struct task *task = state->link.data;

    count = buffer_read(&task->mailbox.buffer, buffer, count);

    if (!count)
        task_setstatus(task, TASK_STATUS_BLOCKED);

    state->offset += count;

    return count;

}

void system_multicast(struct system_node *node, void *buffer, unsigned int count)
{

    struct list_item *current;

    for (current = node->links.head; current; current = current->next)
    {

        struct task *task = current->data;

        task_setstatus(task, TASK_STATUS_UNBLOCKED);
        buffer_write(&task->mailbox.buffer, buffer, count);

    }

}

void system_addchild(struct system_node *group, struct system_node *node)
{

    struct list_item *current;

    if (node->type & SYSTEM_NODETYPE_MULTI)
    {

        unsigned int length0 = ascii_length(node->name);

        for (current = group->children.head; current; current = current->next)
        {

            struct system_node *n = current->data;
            unsigned int length1 = ascii_length(n->name);

            if (length0 != length1 || !memory_match(n->name, node->name, length0))
                continue;

            node->index++;

        }

    }

    list_add(&group->children, &node->item);

    node->parent = group;

}

void system_removechild(struct system_node *group, struct system_node *node)
{

    list_remove(&group->children, &node->item);

    node->parent = 0;

}

void system_initnode(struct system_node *node, unsigned int type, char *name)
{

    list_inititem(&node->item, node);

    node->type = type;
    node->name = name;

    if (type & SYSTEM_NODETYPE_GROUP)
    {

        node->open = system_opengroup;
        node->close = system_closegroup;
        node->child = system_childgroup;

    }

    if (type & SYSTEM_NODETYPE_MAILBOX)
    {

        node->open = system_openmailbox;
        node->close = system_closemailbox;
        node->read = system_readmailbox;

    }

}

void system_initresourcenode(struct system_node *node, unsigned int type, char *name, struct resource *resource)
{

    system_initnode(node, type, name);

    node->resource = resource;

}

void module_init(void)
{

    system_initbackend(&backend);
    system_initprotocol(&protocol);

}

void module_register(void)
{

    resource_register(&backend.resource);
    resource_register(&protocol.resource);

}

void module_unregister(void)
{

    resource_unregister(&backend.resource);
    resource_unregister(&protocol.resource);

}

