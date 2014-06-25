#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include "system.h"
#include "backend.h"
#include "protocol.h"

static struct system_backend backend;
static struct vfs_protocol protocol;

void system_group_add(struct system_group *group, struct system_node *node)
{

    list_add(&group->children, &node->item);

    node->parent = &group->node;

}

void system_group_remove(struct system_group *group, struct system_node *node)
{

    list_remove(&group->children, &node->item);

    node->parent = 0;

}

void system_register_node(struct system_node *node)
{

    system_group_add(&backend.root, node);

}

void system_unregister_node(struct system_node *node)
{

    system_group_remove(&backend.root, node);

}

static unsigned int open(struct system_node *self, unsigned int flags)
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

    struct system_group *group = (struct system_group *)self;
    struct list_item *current;
    unsigned char *b = buffer;
    unsigned int c = 0;

    c += memory_read(b + c, count - c, "../\n", 4, offset);
    offset -= (offset > 4) ? 4 : offset;

    for (current = group->children.head; current; current = current->next)
    {

        struct system_node *node = current->data;
        unsigned int l = ascii_length(node->name);

        c += memory_read(b + c, count - c, node->name, l, offset);
        offset -= (offset > l) ? l : offset;

        if (node->type == SYSTEM_NODETYPE_GROUP)
        {

            c += memory_read(b + c, count - c, "/\n", 2, offset);
            offset -= (offset > 2) ? 2 : offset;

        }

        else
        {

            c += memory_read(b + c, count - c, "\n", 1, offset);
            offset -= (offset > 1) ? 1 : offset;

        }

    }

    return c;

}

unsigned int child_group(struct system_node *self, unsigned int count, const char *path)
{

    struct system_group *group = (struct system_group *)self;
    struct list_item *current;

    if (!count)
        return (unsigned int)self;

    for (current = group->children.head; current; current = current->next)
    {

        struct system_node *node = current->data;
        unsigned int l = ascii_length(node->name);

        if (!memory_match(node->name, path, l))
            continue;

        return (node->type == SYSTEM_NODETYPE_GROUP) ? node->child(node, count - l - 1, path + l + 1) : node->child(node, count - l, path + l);

    }

    return 0;

}

static void system_init_node(struct system_node *node, unsigned int type, const char *name)
{

    memory_clear(node, sizeof (struct system_node));
    list_init_item(&node->item, node);

    node->type = type;
    node->name = name;
    node->open = open;
    node->close = close;
    node->read = read;
    node->write = write;
    node->child = child;

}

void system_init_group(struct system_group *group, const char *name)
{

    memory_clear(group, sizeof (struct system_group));
    system_init_node(&group->node, SYSTEM_NODETYPE_GROUP, name);
    list_init(&group->children);

    group->node.read = read_group;
    group->node.child = child_group;

}

void system_init_stream(struct system_stream *stream, const char *name)
{

    memory_clear(stream, sizeof (struct system_stream));
    system_init_node(&stream->node, SYSTEM_NODETYPE_STREAM, name);

}

void init()
{

    system_init_backend(&backend);
    system_init_protocol(&protocol);
    resource_register(&backend.base.resource);
    resource_register(&protocol.resource);

}

void destroy()
{

}

