#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/event/event.h>
#include "keyboard.h"

static struct event_keypress keypress;

void keyboard_notify(struct keyboard_interface *interface, unsigned int size, unsigned int count, void *buffer)
{

    system_write(&interface->data, 0, size, count, buffer);

}

void keyboard_notifykeypress(struct keyboard_interface *interface, unsigned char scancode)
{

    keypress.scancode = scancode;

    event_notify(sizeof (struct event_keypress), 1, &keypress);

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

void module_init()
{

    keypress.header.type = EVENT_KEYPRESS;
    keypress.header.count = sizeof (struct event_keypress) - sizeof (struct event_header);

}

