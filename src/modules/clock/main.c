#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "clock.h"

static struct system_node root;

void clock_registerinterface(struct clock_interface *interface)
{

    resource_register(&interface->resource);
    system_addchild(&interface->root, &interface->ctrl);
    system_addchild(&root, &interface->root);

}

void clock_unregisterinterface(struct clock_interface *interface)
{

    resource_unregister(&interface->resource);
    system_removechild(&interface->root, &interface->ctrl);
    system_removechild(&root, &interface->root);

}

void clock_initinterface(struct clock_interface *interface, unsigned int id)
{

    resource_init(&interface->resource, RESOURCE_CLOCKINTERFACE, interface);
    system_initnode(&interface->root, SYSTEM_NODETYPE_MULTIGROUP, "if");
    system_initnode(&interface->ctrl, SYSTEM_NODETYPE_NORMAL, "ctrl");

    interface->id = id;

}

void module_init(void)
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "clock");

}

void module_register(void)
{

    system_registernode(&root);

}

void module_unregister(void)
{

    system_unregisternode(&root);

}

