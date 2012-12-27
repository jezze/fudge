#include <kernel/vfs.h>
#include "system.h"
#include "filesystem.h"

static struct system_filesystem filesystem;

void system_register_group(struct system_group *group)
{

    struct system_group *current;

    if (!filesystem.groups)
    {

        filesystem.groups = group;

        return;

    }

    for (current = filesystem.groups; current; current = current->next)
    {

        if (current->next)
            continue;

        current->next = group;

        return;

    }

}

void system_unregister_group(struct system_group *group)
{

    struct system_group *current;

    if (filesystem.groups == group)
    {

        filesystem.groups = filesystem.groups->next;

        return;

    }

    for (current = filesystem.groups; current; current = current->next)
    {

        if (current->next != group)
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

