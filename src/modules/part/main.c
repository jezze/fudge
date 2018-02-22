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

        return node->child(node, state, path, length);

    }

    spinlock_release(&root.children.spinlock);

    return self;

}

static unsigned int partctrl_read(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    struct part *part = self->resource->data;

    return memory_read(buffer, count, &part->settings, sizeof (struct ctrl_partsettings), offset);

}

static unsigned int partctrl_write(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    struct part *part = self->resource->data;

    return memory_write(&part->settings, sizeof (struct ctrl_partsettings), buffer, count, offset);

}

static struct system_node *partdata_open(struct system_node *self, struct service_state *state)
{

    struct part *part = self->resource->data;
    struct block_interface *interface = findinterface(part->settings.interface);

    return interface->data.open(&interface->data, state);

}

static struct system_node *partdata_close(struct system_node *self, struct service_state *state)
{

    struct part *part = self->resource->data;
    struct block_interface *interface = findinterface(part->settings.interface);

    return interface->data.close(&interface->data, state);

}

static unsigned int partdata_read(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    struct part *part = self->resource->data;
    struct block_interface *interface = findinterface(part->settings.interface);

    return interface->data.read(&interface->data, current, state, buffer, count, offset);

}

static unsigned int partdata_write(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    struct part *part = self->resource->data;
    struct block_interface *interface = findinterface(part->settings.interface);

    return interface->data.write(&interface->data, current, state, buffer, count, offset);

}

void part_init(struct part *part)
{

    ctrl_setpartsettings(&part->settings, 0, 0, 0);
    resource_init(&part->resource, RESOURCE_PART, part);
    system_initresourcenode(&part->root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "part", &part->resource);
    system_initresourcenode(&part->ctrl, SYSTEM_NODETYPE_NORMAL, "ctrl", &part->resource);
    system_initresourcenode(&part->data, SYSTEM_NODETYPE_NORMAL, "data", &part->resource);

    part->ctrl.read = partctrl_read;
    part->ctrl.write = partctrl_write;
    part->data.open = partdata_open;
    part->data.close = partdata_close;
    part->data.read = partdata_read;
    part->data.write = partdata_write;

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

    clone.child = clone_child;

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

