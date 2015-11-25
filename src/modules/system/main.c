#include <fudge.h>
#include <kernel.h>
#include "system.h"

static struct vfs_backend backend;
static struct vfs_protocol protocol;

static unsigned int node_openmailboxes(struct system_node *self)
{

    struct task *task = task_findactive();

    if (!list_find(&self->mailboxes, &task->blockitem))
        list_add(&self->mailboxes, &task->blockitem);

    return (unsigned int)self;

}

static unsigned int node_closemailboxes(struct system_node *self)
{

    struct task *task = task_findactive();

    if (list_find(&self->mailboxes, &task->blockitem))
        list_remove(&self->mailboxes, &task->blockitem);

    return (unsigned int)self;

}

static unsigned int node_readgroup(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct record *record = buffer;
    struct system_node *node = (offset) ? (struct system_node *)offset : self->parent;

    record->id = (unsigned int)node;
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

    return sizeof (struct record);

}

static unsigned int node_readmailboxes(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct task *task = task_findactive();

    count = buffer_rcfifo(&task->mailbox.buffer, count, buffer);

    if (!count)
        task_setstatus(task, TASK_STATUS_BLOCKED);

    return count;

}

static unsigned int node_writemailboxes(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct list_item *current;

    for (current = self->mailboxes.head; current; current = current->next)
    {

        struct task *task = current->data;

        task_setstatus(task, TASK_STATUS_ACTIVE);

        return buffer_wcfifo(&task->mailbox.buffer, count, buffer);

    }

    return 0;

}

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

static unsigned int node_scangroup(struct system_node *self, unsigned int index)
{

    if (index)
    {

        struct system_node *n = (struct system_node *)index;

        if (n->item.next)
            return (unsigned int)n->item.next->data;

    }

    else
    {

        if (self->children.head)
            return (unsigned int)self->children.head->data;

    }

    return 0;

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

    if (type & SYSTEM_NODETYPE_MAILBOX)
    {

        node->open = node_openmailboxes;
        node->close = node_closemailboxes;
        node->read = node_readmailboxes;
        node->write = node_writemailboxes;

    }

    if (type & SYSTEM_NODETYPE_GROUP)
    {

        node->read = node_readgroup;
        node->child = node_childgroup;
        node->scan = node_scangroup;

    }

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

