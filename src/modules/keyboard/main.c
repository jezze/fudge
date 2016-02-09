#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "keyboard.h"

void keyboard_notify(struct keyboard_interface *interface, unsigned int count, void *buffer)
{

    system_multicast(&interface->data.links, count, buffer);

}

void keyboard_registerinterface(struct keyboard_interface *interface, unsigned int id)
{

    resource_register(&interface->resource);
    system_addchild(&interface->root, &interface->data);
    system_registernode(&interface->root);

    interface->id = id;

}

void keyboard_unregisterinterface(struct keyboard_interface *interface)
{

    resource_unregister(&interface->resource);
    system_removechild(&interface->root, &interface->data);
    system_unregisternode(&interface->root);

}

void keyboard_initinterface(struct keyboard_interface *interface)
{

    resource_init(&interface->resource, RESOURCE_KEYBOARDINTERFACE, interface);
    system_initnode(&interface->root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "keyboard");
    system_initnode(&interface->data, SYSTEM_NODETYPE_MAILBOX, "data");

    interface->data.resource = &interface->resource;

}

