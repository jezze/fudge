#include <fudge/module.h>
#include <kernel/vfs.h>
#include "system.h"
#include "filesystem.h"

static struct system_filesystem filesystem;

void system_group_add(struct system_group *group, struct system_node *node)
{

    struct system_node *current;

    node->parent = &group->node;

    if (!group->children)
    {

        group->children = node;

        return;

    }

    for (current = group->children; current; current = current->sibling)
    {

        if (current->sibling)
            continue;

        current->sibling = node;

        return;

    }

}

void system_group_remove(struct system_group *group, struct system_node *node)
{

    struct system_node *current;

    node->parent = 0;

    if (group->children == node)
    {

        group->children = group->children->sibling;

        return;

    }

    for (current = group->children; current; current = current->sibling)
    {

        if (current->sibling != node)
            continue;

        current->sibling = current->sibling->sibling;

        return;

    }

}

void system_register_node(struct system_node *node)
{

    system_group_add(&filesystem.root, node);

}

void system_unregister_node(struct system_node *node)
{

    system_group_remove(&filesystem.root, node);

}

static void system_init_node(struct system_node *node, unsigned int type, char *name)
{

    memory_clear(node, sizeof (struct system_node));

    node->type = type;
    node->name = name;

}

void system_init_group(struct system_group *group, char *name)
{

    memory_clear(group, sizeof (struct system_group));
    system_init_node(&group->node, SYSTEM_NODE_TYPE_GROUP, name);

}

void system_init_integer(struct system_integer *integer, char *name, int *value)
{

    memory_clear(integer, sizeof (struct system_integer));
    system_init_node(&integer->node, SYSTEM_NODE_TYPE_INTEGER, name);

    integer->value = value;

}

void system_init_string(struct system_string *string, char *name, char *value)
{

    memory_clear(string, sizeof (struct system_string));
    system_init_node(&string->node, SYSTEM_NODE_TYPE_STRING, name);

    string->value = value;

}

void system_init_stream(struct system_stream *stream, char *name, unsigned int (*read)(struct system_stream *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct system_stream *self, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(stream, sizeof (struct system_stream));
    system_init_node(&stream->node, SYSTEM_NODE_TYPE_STREAM, name);

    stream->read = read;
    stream->write = write;

}

struct vfs_interface *get_filesystem()
{

    return &filesystem.base;

}

void init()
{

    system_init_filesystem(&filesystem);

}

void destroy()
{

}

