#include <fudge.h>
#include <kernel.h>
#include "system.h"
#include "backend.h"
#include "protocol.h"

static struct vfs_backend backend;
static struct vfs_protocol protocol;

static unsigned int read_normal(struct system_node *self, unsigned int offset, unsigned int size, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int read_group(struct system_node *self, unsigned int offset, unsigned int size, unsigned int count, void *buffer)
{

    struct list_item *current;
    struct record *records = buffer;
    unsigned int i = 0;

    if (offset > 0)
        return 0;

    records[i].size = 0;
    records[i].length = memory_read(records[i].name, RECORD_NAMESIZE, "../", 3, 1, 0);

    i++;

    for (current = self->children.head; current; current = current->next)
    {

        struct system_node *node = current->data;

        records[i].size = 0;
        records[i].length = memory_read(records[i].name, RECORD_NAMESIZE, node->name, ascii_length(node->name), 1, 0);

        if (node->type & SYSTEM_NODETYPE_MULTI)
        {

            char *index = ":0";

            index[1] = '0' + node->index;

            records[i].length += memory_write(records[i].name, RECORD_NAMESIZE, index, 2, 1, records[i].length);

        }

        if (node->type & SYSTEM_NODETYPE_GROUP)
            records[i].length += memory_write(records[i].name, RECORD_NAMESIZE, "/", 1, 1, records[i].length);

        i++;

    }

    return i;

}

static unsigned int read_mailboxes(struct system_node *self, unsigned int offset, unsigned int size, unsigned int count, void *buffer)
{

    return task_rmessage(scheduler_findactive(), size, count, buffer);

}

static unsigned int write_normal(struct system_node *self, unsigned int offset, unsigned int size, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int write_mailboxes(struct system_node *self, unsigned int offset, unsigned int size, unsigned int count, void *buffer)
{

    struct list_item *current;

    for (current = self->mailboxes.head; current; current = current->next)
    {

        struct task *task = current->data;

        task_wmessage(task, size, count, buffer);

    }

    return 0;

}

static unsigned int child_normal(struct system_node *self, unsigned int count, const char *path)
{

    if (!count)
        return (unsigned int)self;

    return 0;

}

static unsigned int child_group(struct system_node *self, unsigned int count, const char *path)
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

            if (path[length] == ':')
            {

                unsigned int val = path[length + 1] - '0';

                if (val != node->index)
                    continue;

                length += 2;

            }

            else
            {

                if (node->refcount)
                    continue;

            }

        }

        if (node->type & SYSTEM_NODETYPE_GROUP)
            length += 1;

        return node->child(node, count - length, path + length);

    }

    return 0;

}

static unsigned int open_mailboxes(struct system_node *self)
{

    struct task *task = scheduler_findactive();

    list_add(&self->mailboxes, &task->mailbox.item);

    return system_open(self);

}

static unsigned int close_mailboxes(struct system_node *self)
{

    struct task *task = scheduler_findactive();

    list_remove(&self->mailboxes, &task->mailbox.item);

    return system_close(self);

}

unsigned int system_open(struct system_node *node)
{

    node->refcount++;
    node->parent->refcount++;

    return (unsigned int)node;

}


unsigned int system_close(struct system_node *node)
{

    node->refcount--;
    node->parent->refcount--;

    return (unsigned int)node;

}

unsigned int system_read(struct system_node *node, unsigned int offset, unsigned int size, unsigned int count, void *buffer)
{

    return node->read(node, offset, size, count, buffer);

}

unsigned int system_write(struct system_node *node, unsigned int offset, unsigned int size, unsigned int count, void *buffer)
{

    return node->write(node, offset, size, count, buffer);

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

void system_initnode(struct system_node *node, unsigned int type, const char *name)
{

    list_inititem(&node->item, node);
    list_init(&node->children);
    list_init(&node->mailboxes);

    node->type = type;
    node->name = name;

    if (type & SYSTEM_NODETYPE_MAILBOX)
        node->open = open_mailboxes;
    else
        node->open = system_open;

    if (type & SYSTEM_NODETYPE_MAILBOX)
        node->close = close_mailboxes;
    else
        node->close = system_close;

    if (type & SYSTEM_NODETYPE_GROUP)
        node->read = read_group;
    else if (type & SYSTEM_NODETYPE_MAILBOX)
        node->read = read_mailboxes;
    else
        node->read = read_normal;

    if (type & SYSTEM_NODETYPE_MAILBOX)
        node->write = write_mailboxes;
    else
        node->write = write_normal;

    if (type & SYSTEM_NODETYPE_GROUP)
        node->child = child_group;
    else
        node->child = child_normal;

}

void module_init()
{

    system_initbackend(&backend);
    system_initprotocol(&protocol);

}

void module_register()
{

    resource_register(&backend.resource);
    resource_register(&protocol.resource);

}

void module_unregister()
{

    resource_unregister(&backend.resource);
    resource_unregister(&protocol.resource);

}

