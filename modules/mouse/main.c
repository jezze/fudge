#include <fudge.h>
#include <kernel.h>
#include <system/system.h>
#include <event/event.h>
#include "mouse.h"

void mouse_notify(struct mouse_interface *interface, unsigned int size, unsigned int count, void *buffer)
{

    scheduler_sendlist(&interface->data.mailboxes, size, count, buffer);
    event_notify(EVENT_MOUSE, size, count, buffer);

}

void mouse_registerinterface(struct mouse_interface *interface, unsigned int id)
{

    system_addchild(&interface->root, &interface->data);
    system_registernode(&interface->root);

    interface->id = id;

}

void mouse_unregisterinterface(struct mouse_interface *interface)
{

    system_removechild(&interface->root, &interface->data);
    system_unregisternode(&interface->root);

}

void mouse_initinterface(struct mouse_interface *interface)
{

    resource_init(&interface->resource, 0, interface);
    system_initnode(&interface->root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "mouse");
    system_initnode(&interface->data, SYSTEM_NODETYPE_MAILBOX, "data");

    interface->data.resource = &interface->resource;

}

