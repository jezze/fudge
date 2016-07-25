#include <fudge.h>
#include <kernel.h>
#include "system.h"

static struct service_backend backend;
static struct service_protocol protocol;

static unsigned int node_childgroup(struct system_node *self, unsigned int count, char *path)
{

    struct list_item *current;

    if (!count)
        return (unsigned int)self;

    for (current = self->children.head; current; current = current->next)
    {

        struct system_node *node = current->data;
        unsigned int length = ascii_length(node->name);

        if (!memory_match(node->name, path, length))
            continue;

        if (node->type & SYSTEM_NODETYPE_MULTI)
        {

            unsigned int val;

            if (path[length] != ':')
                continue;

            val = path[length + 1] - '0';

            if (val != node->index)
                continue;

            length += 2;

        }

        if (node->type & SYSTEM_NODETYPE_GROUP)
            length += 1;

        count -= length;
        path += length;

        return (node->child) ? node->child(node, count, path) : (count ? 0 : (unsigned int)node);

    }

    return 0;

}

unsigned int system_send(unsigned int id, unsigned int count, void *buffer)
{

    struct task *destination = (struct task *)id;

    if (destination->mailbox.buffer.capacity - destination->mailbox.buffer.count < count)
        return 0;

    task_setstatus(destination, TASK_STATUS_UNBLOCKED);

    return buffer_wcfifo(&destination->mailbox.buffer, count, buffer);

}

void system_multicast(struct list *list, unsigned int count, void *buffer)
{

    struct list_item *current;

    for (current = list->head; current; current = current->next)
    {

        struct task *task = current->data;

        task_setstatus(task, TASK_STATUS_UNBLOCKED);
        buffer_wcfifo(&task->mailbox.buffer, count, buffer);

    }

}

void system_wakeup(struct list *list)
{

    struct list_item *current;

    for (current = list->head; current; current = current->next)
    {

        struct task *task = current->data;

        list_remove(list, current);
        task_setstatus(task, TASK_STATUS_UNBLOCKED);

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

        node->child = node_childgroup;

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

