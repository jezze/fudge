#include <fudge/memory.h>
#include <kernel/vfs.h>
#include "system.h"
#include "filesystem.h"

static struct system_filesystem filesystem;

void system_register_node(struct system_node *node)
{

    struct system_node *current;

    if (!filesystem.root.children)
    {

        filesystem.root.children = node;

        return;

    }

    for (current = filesystem.root.children; current; current = current->next)
    {

        if (current->next)
            continue;

        current->next = node;

        return;

    }

}

void system_unregister_node(struct system_node *node)
{

    struct system_node *current;

    if (filesystem.root.children == node)
    {

        filesystem.root.children = filesystem.root.children->next;

        return;

    }

    for (current = filesystem.root.children; current; current = current->next)
    {

        if (current->next != node)
            continue;

        current->next = current->next->next;

        return;

    }

}

void system_register_routine(unsigned int index, unsigned int (*reader)(struct system_node *node, unsigned int offset, unsigned int count, void *buffer), unsigned int (*writer)(struct system_node *node, unsigned int offset, unsigned int count, void *buffer))
{

    filesystem.readers[index] = reader;
    filesystem.writers[index] = writer;

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

    system_init_node(&group->base, SYSTEM_NODE_TYPE_GROUP, name);

}

void system_init_integer(struct system_integer *integer, char *name, int value)
{

    memory_clear(integer, sizeof (struct system_integer));

    system_init_node(&integer->base, SYSTEM_NODE_TYPE_INTEGER, name);

    integer->value = value;

}

void system_init_string(struct system_string *string, char *name, char *value)
{

    memory_clear(string, sizeof (struct system_string));

    system_init_node(&string->base, SYSTEM_NODE_TYPE_STRING, name);

    string->value = value;

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

