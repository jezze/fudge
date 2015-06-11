#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "timer.h"

void timer_notify(struct timer_interface *interface, unsigned int size, unsigned int count, void *buffer)
{

    scheduler_sendlist(&interface->sleep.mailboxes, size, count, buffer);

}

void timer_registerinterface(struct timer_interface *interface, unsigned int id)
{

    system_addchild(&interface->root, &interface->sleep);
    system_registernode(&interface->root);

}

void timer_unregisterinterface(struct timer_interface *interface)
{

    system_removechild(&interface->root, &interface->sleep);
    system_unregisternode(&interface->root);

}

void timer_initinterface(struct timer_interface *interface)
{

    resource_init(&interface->resource, 0, interface);
    system_initnode(&interface->root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "timer");
    system_initnode(&interface->sleep, SYSTEM_NODETYPE_MAILBOX, "sleep");

    interface->sleep.resource = &interface->resource;

}

