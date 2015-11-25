#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "timer.h"

void timer_notify(struct timer_interface *interface, unsigned int count, void *buffer)
{

    system_multicast(&interface->sleep, count, buffer);

}

void timer_registerinterface(struct timer_interface *interface, unsigned int id)
{

    resource_register(&interface->resource);
    system_addchild(&interface->root, &interface->sleep);
    system_registernode(&interface->root);

    interface->id = id;

}

void timer_unregisterinterface(struct timer_interface *interface)
{

    resource_unregister(&interface->resource);
    system_removechild(&interface->root, &interface->sleep);
    system_unregisternode(&interface->root);

}

void timer_initinterface(struct timer_interface *interface)
{

    resource_init(&interface->resource, RESOURCE_TIMERINTERFACE, interface);
    system_initnode(&interface->root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "timer");
    system_initnode(&interface->sleep, SYSTEM_NODETYPE_MAILBOX, "sleep");

    interface->sleep.resource = &interface->resource;

}

