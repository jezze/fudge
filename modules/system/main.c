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

