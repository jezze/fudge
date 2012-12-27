#include <kernel/vfs.h>
#include <base/base.h>
#include "nodefs.h"

static struct nodefs_filesystem filesystem;

static void nodefs_register_node2(struct nodefs_node *node)
{

    struct nodefs_node *current;

    if (!filesystem.nodes)
    {

        filesystem.nodes = node;

        return;

    }

    for (current = filesystem.nodes; current; current = current->next)
    {

        if (current->next)
            continue;

        current->next = node;

        return;

    }

}

void nodefs_register_node(struct nodefs_node *node, char *name, struct base_module *module, unsigned int (*read)(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer))
{

    nodefs_register_node2(node);

    node->name = name;
    node->module = module;
    node->read = read;
    node->write = write;

}

void nodefs_unregister_node(struct nodefs_node *node)
{

    struct nodefs_node *current;

    if (filesystem.nodes == node)
    {

        filesystem.nodes = filesystem.nodes->next;

        return;

    }

    for (current = filesystem.nodes; current; current = current->next)
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

    nodefs_init_filesystem(&filesystem);

}

void destroy()
{

}

