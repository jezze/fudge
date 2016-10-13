#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/block/block.h>
#include "part.h"

static struct system_node root;
static struct system_node clone;

static struct block_interface *findinterface(unsigned int index)
{

    struct resource *resource = 0;

    while ((resource = resource_findtype(resource, RESOURCE_BLOCKINTERFACE)))
    {

        if (!index)
            return resource->data;

        index--;

    }

    return 0;

}

static unsigned int clone_child(struct system_node *self, char *path, unsigned int length)
{

    struct list_item *current;

    for (current = root.children.head; current; current = current->next)
    {

        struct system_node *node = current->data;

        if (node == self)
            continue;

        return node->child(node, path, length);

    }

    return 0;

}

static unsigned int ctrl_read(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    struct part *part = (struct part *)self->parent;

    return memory_read(buffer, count, &part->settings, sizeof (struct ctrl_partsettings), state->offset);

}

static unsigned int ctrl_write(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    struct part *part = (struct part *)self->parent;

    count = memory_write(&part->settings, sizeof (struct ctrl_partsettings), buffer, count, state->offset);
    part->parent = findinterface(part->settings.interface);

    return count;

}

void part_init(struct part *part)
{

    ctrl_setpartsettings(&part->settings, 0, 0, 0);
    system_initnode(&part->root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "part");
    system_initnode(&part->ctrl, SYSTEM_NODETYPE_NORMAL, "ctrl");

    part->ctrl.read = ctrl_read;
    part->ctrl.write = ctrl_write;

}

void part_register(struct part *part)
{

    system_addchild(&part->root, &part->ctrl);
    system_addchild(&root, &part->root);

}

void part_unregister(struct part *part)
{

    system_removechild(&part->root, &part->ctrl);
    system_removechild(&root, &part->root);

}

void module_init(void)
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "part");
    system_initnode(&clone, SYSTEM_NODETYPE_GROUP, "clone");

    clone.child = clone_child;

    system_addchild(&root, &clone);

}

void module_register(void)
{

    system_registernode(&root);

}

void module_unregister(void)
{

    system_unregisternode(&root);

}

