#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include "system.h"
#include "backend.h"
#include "protocol.h"

static struct system_backend backend;
static struct vfs_protocol protocol;

void system_addchild(struct system_node *group, struct system_node *node)
{

    struct list_item *current;

    if (node->multi)
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

void system_registernode(struct system_node *node)
{

    system_addchild(&backend.root, node);

}

void system_unregisternode(struct system_node *node)
{

    system_removechild(&backend.root, node);

}

static unsigned int open(struct system_node *self)
{

    return (unsigned int)self;

}

static unsigned int close(struct system_node *self)
{

    return (unsigned int)self;

}

static unsigned int read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

unsigned int child(struct system_node *self, unsigned int count, const char *path)
{

    if (!count)
        return (unsigned int)self;

    return 0;

}

static unsigned int read_group(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct list_item *current;
    unsigned char *b = buffer;
    unsigned int c = 0;

    c += memory_read(b + c, count - c, "../\n", 4, offset);
    offset -= (offset > 4) ? 4 : offset;

    for (current = self->children.head; current; current = current->next)
    {

        struct system_node *node = current->data;
        unsigned int l = ascii_length(node->name);

        c += memory_read(b + c, count - c, node->name, l, offset);
        offset -= (offset > l) ? l : offset;

        if (node->multi)
        {

            char *x = ":0";

            x[1] = '0' + node->index;

            c += memory_read(b + c, count - c, x, 2, offset);
            offset -= (offset > 2) ? 2 : offset;

        }

        if (node->type == SYSTEM_NODETYPE_GROUP)
        {

            c += memory_read(b + c, count - c, "/", 1, offset);
            offset -= (offset > 1) ? 1 : offset;

        }

        c += memory_read(b + c, count - c, "\n", 1, offset);
        offset -= (offset > 1) ? 1 : offset;

    }

    return c;

}

unsigned int child_group(struct system_node *self, unsigned int count, const char *path)
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

        if (node->multi)
        {

            unsigned int val = path[length + 1] - '0';

            if (val != node->index)
                continue;

            length += 2;

        }

        if (node->type == SYSTEM_NODETYPE_GROUP)
            length += 1;

        return node->child(node, count - length, path + length);

    }

    return 0;

}

void system_initnode(struct system_node *node, unsigned int type, const char *name)
{

    memory_clear(node, sizeof (struct system_node));
    list_inititem(&node->item, node);
    list_init(&node->children);

    node->type = type;
    node->name = name;
    node->open = open;
    node->close = close;
    node->read = read;
    node->write = write;
    node->child = child;

}

void system_initstream(struct system_node *stream, const char *name)
{

    system_initnode(stream, SYSTEM_NODETYPE_STREAM, name);

}

void system_initmultistream(struct system_node *stream, const char *name)
{

    system_initstream(stream, name);

    stream->multi = 1;

}

void system_initgroup(struct system_node *group, const char *name)
{

    system_initnode(group, SYSTEM_NODETYPE_GROUP, name);

    group->read = read_group;
    group->child = child_group;

}

void system_initmultigroup(struct system_node *group, const char *name)
{

    system_initgroup(group, name);

    group->multi = 1;

}

void init()
{

    system_initbackend(&backend);
    system_initprotocol(&protocol);
    resource_register(&backend.base.resource);
    resource_register(&protocol.resource);

}

void destroy()
{

    resource_unregister(&backend.base.resource);
    resource_unregister(&protocol.resource);

}

