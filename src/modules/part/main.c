#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/block/block.h>
#include "part.h"

static struct system_node root;
static struct system_node clone;

struct block_interface *part_findinterface(unsigned int index)
{

    struct resource *resource = 0;

    while ((resource = resource_foreachtype(resource, RESOURCE_BLOCKINTERFACE)))
    {

        if (!index)
            return resource->data;

        index--;

    }

    return 0;

}

static struct system_node *clone_child(struct system_node *self, struct service_state *state, char *path, unsigned int length)
{

    struct list_item *current;

    spinlock_acquire(&root.children.spinlock);

    for (current = root.children.head; current; current = current->next)
    {

        struct system_node *node = current->data;

        if (node == self)
            continue;

        return node->operations.child(node, state, path, length);

    }

    spinlock_release(&root.children.spinlock);

    return self;

}

void part_init(struct part *part)
{

    ctrl_setpartsettings(&part->settings, 0, 0, 0);
    resource_init(&part->resource, RESOURCE_PART, part);
    system_initnode(&part->root, SYSTEM_NODETYPE_MULTIGROUP, "part");
    system_initnode(&part->ctrl, SYSTEM_NODETYPE_NORMAL, "ctrl");
    system_initnode(&part->data, SYSTEM_NODETYPE_NORMAL, "data");

}

void part_register(struct part *part)
{

    system_addchild(&part->root, &part->ctrl);
    system_addchild(&part->root, &part->data);
    system_addchild(&root, &part->root);

}

void part_unregister(struct part *part)
{

    system_removechild(&part->root, &part->ctrl);
    system_removechild(&part->root, &part->data);
    system_removechild(&root, &part->root);

}

void module_init(void)
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "part");
    system_initnode(&clone, SYSTEM_NODETYPE_GROUP, "clone");

    clone.operations.child = clone_child;

}

void module_register(void)
{

    system_registernode(&root);
    system_addchild(&root, &clone);

}

void module_unregister(void)
{

    system_unregisternode(&root);
    system_removechild(&root, &clone);

}

