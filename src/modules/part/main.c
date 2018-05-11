#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/block/block.h>
#include "part.h"

static struct system_node root;

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

}

void module_register(void)
{

    system_registernode(&root);

}

void module_unregister(void)
{

    system_unregisternode(&root);

}

