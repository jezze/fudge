#include <fudge.h>
#include <kernel.h>
#include "system.h"

static struct system_node root;
static struct service_protocol protocol;

static struct system_node *getnode(unsigned int id)
{

    return (struct system_node *)id;

}

static unsigned int protocol_root(void)
{

    return (unsigned int)&root;

}

static unsigned int protocol_parent(unsigned int id)
{

    struct system_node *node = getnode(id);

    return (unsigned int)node->parent;

}

static unsigned int protocol_child(unsigned int id, char *path, unsigned int length)
{

    struct system_node *node = getnode(id);
    struct list_item *current;
    struct system_node *n = node;

    spinlock_acquire(&node->children.spinlock);

    for (current = node->children.head; current; current = current->next)
    {

        struct system_node *n2 = current->data;
        unsigned int length0 = ascii_length(n2->name);

        if (n2->type == SYSTEM_NODETYPE_MULTIGROUP)
        {

            unsigned int colon = buffer_findbyte(path, length, ':');
            unsigned int val;

            if (length0 != colon)
                continue;

            if (!buffer_match(n2->name, path, colon))
                continue;

            val = ascii_rvalue(path + colon + 1, length - colon - 1, 10);

            if (val != n2->index)
                continue;

        }

        else
        {

            if (length0 != length)
                continue;

            if (!buffer_match(n2->name, path, length))
                continue;

        }

        n = n2;

        break;

    }

    spinlock_release(&node->children.spinlock);

    return (unsigned int)n;

}

static unsigned int protocol_create(unsigned int id, char *name, unsigned int length)
{

    struct system_node *node = getnode(id);

    return (node->operations.create) ? node->operations.create(name, length) : 0;

}

static unsigned int protocol_destroy(unsigned int id)
{

    struct system_node *node = getnode(id);

    return (node->operations.destroy) ? node->operations.destroy() : 0;

}

static unsigned int protocol_step(unsigned int id, unsigned int current)
{

    struct system_node *node = getnode(current);

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

static unsigned int protocol_open(unsigned int id)
{

    return id;

}

static unsigned int protocol_close(unsigned int id)
{

    return id;

}

static unsigned int readgroup(struct system_node *current, void *buffer, unsigned int count, unsigned int offset)
{

    struct record record;

    if (!current)
        return 0;

    record.id = (unsigned int)current;
    record.size = 0;
    record.length = buffer_read(record.name, RECORD_NAMESIZE, current->name, ascii_length(current->name), 0);

    if (current->type == SYSTEM_NODETYPE_MULTIGROUP)
    {

        char num[ASCII_NUMSIZE];

        record.length += buffer_write(record.name, RECORD_NAMESIZE, ":", 1, record.length);
        record.length += buffer_write(record.name, RECORD_NAMESIZE, num, ascii_wvalue(num, ASCII_NUMSIZE, current->index, 10, 0), record.length);

    }

    return buffer_read(buffer, count, &record, sizeof (struct record), offset);

}

static unsigned int protocol_read(unsigned int id, unsigned int current, void *buffer, unsigned int count, unsigned int offset)
{

    struct system_node *node = getnode(id);
    struct system_node *currentnode = getnode(current);

    switch (node->type)
    {

    case SYSTEM_NODETYPE_GROUP:
    case SYSTEM_NODETYPE_MULTIGROUP:
        return readgroup(currentnode, buffer, count, offset);

    }

    return (node->operations.read) ? node->operations.read(buffer, count, offset) : 0;

}

static unsigned int protocol_write(unsigned int id, unsigned int current, void *buffer, unsigned int count, unsigned int offset)
{

    struct system_node *node = getnode(id);

    return (node->operations.write) ? node->operations.write(buffer, count, offset) : 0;

}

static unsigned int protocol_seek(unsigned int id, unsigned int offset)
{

    struct system_node *node = getnode(id);

    return (node->operations.seek) ? node->operations.seek(offset) : offset;

}

static unsigned int protocol_map(unsigned int id)
{

    return id;

}

static unsigned int protocol_link(unsigned int id, unsigned int source)
{

    struct system_node *node = getnode(id);
    struct link *link = kernel_picklink(source);

    if (link)
        list_add(&node->links, &link->item);

    return id;

}

static unsigned int protocol_unlink(unsigned int id, unsigned int source)
{

    struct system_node *node = getnode(id);
    struct list_item *current;

    spinlock_acquire(&node->links.spinlock);

    for (current = node->links.head; current; current = current->next)
    {

        struct link *link = current->data;

        if (link->source == source)
        {

            list_remove_nolock(&node->links, &link->item);
            kernel_freelink(link);

        }

    }

    spinlock_release(&node->links.spinlock);

    return id;

}

static unsigned int protocol_notify(unsigned int id, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    struct system_node *node = getnode(id);
    struct message_header header;
    struct list_item *current;

    message_initheader(&header, event, count);
    spinlock_acquire(&node->links.spinlock);

    for (current = node->links.head; current; current = current->next)
    {

        struct link *target = current->data;

        kernel_place(source, target->source, &header, data);

    }

    spinlock_release(&node->links.spinlock);

    return count;

}

void system_registernode(struct system_node *node)
{

    system_addchild(&root, node);

}

void system_unregisternode(struct system_node *node)
{

    system_removechild(&root, node);

}

void system_addchild(struct system_node *group, struct system_node *node)
{

    struct list_item *current;

    if (node->type == SYSTEM_NODETYPE_MULTIGROUP)
    {

        unsigned int length0 = ascii_length(node->name);

        spinlock_acquire(&group->children.spinlock);

        for (current = group->children.head; current; current = current->next)
        {

            struct system_node *n = current->data;
            unsigned int length1 = ascii_length(n->name);

            if (length0 != length1 || !buffer_match(n->name, node->name, length0))
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
    list_init(&node->children);
    list_init(&node->links);

    node->type = type;
    node->name = name;
    node->index = 0;
    node->parent = 0;
    node->operations.create = 0;
    node->operations.destroy = 0;
    node->operations.read = 0;
    node->operations.write = 0;
    node->operations.seek = 0;

}

void module_init(void)
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "FUDGE_ROOT");

    root.parent = &root;

    service_initprotocol(&protocol, "system", protocol_root, protocol_parent, protocol_child, protocol_create, protocol_destroy, protocol_step, protocol_open, protocol_close, protocol_read, protocol_write, protocol_seek, protocol_map, protocol_link, protocol_unlink, protocol_notify);
    resource_register(&protocol.resource);

}

