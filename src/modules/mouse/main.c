#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "mouse.h"

void mouse_notify(struct mouse_interface *interface, unsigned int count, void *buffer)
{

    system_multicast(&interface->data.links, count, buffer);

}

void mouse_registerinterface(struct mouse_interface *interface, unsigned int id)
{

    resource_register(&interface->resource);
    system_addchild(&interface->root, &interface->data);
    system_registernode(&interface->root);

    interface->id = id;

}

void mouse_unregisterinterface(struct mouse_interface *interface)
{

    resource_unregister(&interface->resource);
    system_removechild(&interface->root, &interface->data);
    system_unregisternode(&interface->root);

}

void mouse_initinterface(struct mouse_interface *interface)
{

    resource_init(&interface->resource, RESOURCE_MOUSEINTERFACE, interface);
    system_initnode(&interface->root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "mouse");
    system_initnode(&interface->data, SYSTEM_NODETYPE_MAILBOX, "data");

    interface->data.resource = &interface->resource;

}

