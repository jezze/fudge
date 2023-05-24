#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "block.h"

static struct system_node root;

void block_notify(struct block_interface *interface, void *buffer, unsigned int count)
{

    kernel_notify(&interface->data.links, EVENT_DATA, count, buffer);

}

void block_registerinterface(struct block_interface *interface)
{

    resource_register(&interface->resource);
    system_addchild(&interface->root, &interface->data);
    system_addchild(&root, &interface->root);

}

void block_unregisterinterface(struct block_interface *interface)
{

    resource_unregister(&interface->resource);
    system_removechild(&interface->root, &interface->data);
    system_removechild(&root, &interface->root);

}

void block_initinterface(struct block_interface *interface, unsigned int id)
{

    resource_init(&interface->resource, RESOURCE_BLOCKINTERFACE, interface);
    system_initnode(&interface->root, SYSTEM_NODETYPE_MULTIGROUP, "if");
    system_initnode(&interface->data, SYSTEM_NODETYPE_NORMAL, "data");

    interface->id = id;

}

void module_init(void)
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "block");

}

void module_register(void)
{

    system_registernode(&root);

}

void module_unregister(void)
{

    system_unregisternode(&root);

}

