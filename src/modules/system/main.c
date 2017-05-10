#include <fudge.h>
#include <kernel.h>
#include "system.h"

static struct service_backend backend;
static struct service_protocol protocol;

unsigned int system_childgroup(struct system_node *self, struct service_state *state, char *path, unsigned int length)
{

    struct list_item *current;

    for (current = self->children.head; current; current = current->next)
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

        }

        return (unsigned int)node;

    }

    return 0;

}

unsigned int system_readtask(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    return kernel_readtask(state->task, buffer, count);

}

unsigned int system_readgroup(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    struct record record;
    struct system_node *current = (struct system_node *)state->current;

    if (!state->current)
        return 0;

    record.id = state->current;
    record.size = 0;
    record.length = memory_read(record.name, RECORD_NAMESIZE, current->name, ascii_length(current->name), 0);

    if (current->type & SYSTEM_NODETYPE_MULTI)
    {

        char num[FUDGE_NSIZE];

        record.length += memory_write(record.name, RECORD_NAMESIZE, ":", 1, record.length);
        record.length += memory_write(record.name, RECORD_NAMESIZE, num, ascii_wvalue(num, FUDGE_NSIZE, current->index, 10, 0), record.length);

    }

    return memory_read(buffer, count, &record, sizeof (struct record), state->offset);

}

unsigned int system_seek(struct system_node *self, struct service_state *state, unsigned int offset)
{

    return offset;

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

        node->read = system_readgroup;
        node->child = system_childgroup;
        node->seek = system_seek;

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

