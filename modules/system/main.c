#include <kernel/vfs.h>
#include "system.h"
#include "filesystem.h"

static struct system_filesystem filesystem;

void system_register_group(struct system_group *group)
{

    struct system_node *current;

    if (!filesystem.groups)
    {

        filesystem.groups = group;

        return;

    }

    for (current = &filesystem.groups->base; current; current = current->next)
    {

        if (current->next)
            continue;

        current->next = &group->base;

        return;

    }

}

void system_unregister_group(struct system_group *group)
{

    struct system_node *current;

    if (filesystem.groups == group)
    {

        filesystem.groups = (struct system_group *)filesystem.groups->base.next;

        return;

    }

    for (current = &filesystem.groups->base; current; current = current->next)
    {

        if (current->next != &group->base)
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

