#include <fudge.h>
#include <kernel.h>
#include "system.h"

static struct service_backend backend;
static struct service_protocol protocol;

static struct system_node *childgroup(struct system_node *self, struct service_state *state, char *path, unsigned int length)
{

    struct list_item *current;
    struct system_node *n = self;

    spinlock_acquire(&self->children.spinlock);

    for (current = self->children.head; current; current = current->next)
    {

        struct system_node *node = current->data;
        unsigned int length0 = ascii_length(node->name);

        if (node->type & SYSTEM_NODETYPE_MULTI)
        {

            unsigned int colon = memory_findbyte(path, length, ':');
            unsigned int val;

            if (length0 != colon)
                continue;

            if (!memory_match(node->name, path, colon))
                continue;

            val = ascii_rvalue(path, length, 10, colon + 1);

            if (val != node->index)
                continue;

        }

        else
        {

            if (length0 != length)
                continue;

            if (!memory_match(node->name, path, length))
                continue;

        }

        n = node;

        break;

    }

    spinlock_release(&self->children.spinlock);

    return n;

}

static unsigned int readgroup(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    struct record record;

    if (!current)
        return 0;

    record.id = (unsigned int)current;
    record.size = 0;
    record.length = memory_read(record.name, RECORD_NAMESIZE, current->name, ascii_length(current->name), 0);

    if (current->type & SYSTEM_NODETYPE_MULTI)
    {

        char num[FUDGE_NSIZE];

        record.length += memory_write(record.name, RECORD_NAMESIZE, ":", 1, record.length);
        record.length += memory_write(record.name, RECORD_NAMESIZE, num, ascii_wvalue(num, FUDGE_NSIZE, current->index, 10), record.length);

    }

    return memory_read(buffer, count, &record, sizeof (struct record), offset);

}

static unsigned int readmailbox(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    count = kernel_readmailbox(state->task, buffer, count);

    if (!count)
        kernel_blocktask(state->task);

    return count;

}

static unsigned int seeknormal(struct system_node *self, struct service_state *state, unsigned int offset)
{

    return offset;

}

static unsigned int seekmailbox(struct system_node *self, struct service_state *state, unsigned int offset)
{

    return 0;

}

void system_addchild(struct system_node *group, struct system_node *node)
{

    struct list_item *current;

    if (node->type & SYSTEM_NODETYPE_MULTI)
    {

        unsigned int length0 = ascii_length(node->name);

        spinlock_acquire(&group->children.spinlock);

        for (current = group->children.head; current; current = current->next)
        {

            struct system_node *n = current->data;
            unsigned int length1 = ascii_length(n->name);

            if (length0 != length1 || !memory_match(n->name, node->name, length0))
                continue;

            node->index++;

        }

        spinlock_release(&group->children.spinlock);

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
    node->seek = seeknormal;

    if (type & SYSTEM_NODETYPE_MAILBOX)
    {

        node->read = readmailbox;
        node->seek = seekmailbox;

    }

    if (type & SYSTEM_NODETYPE_GROUP)
    {

        node->read = readgroup;
        node->child = childgroup;

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

