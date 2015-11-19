#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/event/event.h>
#include "mouse.h"

static struct event_mousepress mousepress;
static struct event_mouserelease mouserelease;
static struct event_mousemove mousemove;

void mouse_notify(struct mouse_interface *interface, unsigned int count, void *buffer)
{

    interface->data.write(&interface->data, 0, count, buffer);

}

void mouse_notifymousepress(struct mouse_interface *interface, unsigned int button)
{

    mousepress.button = button;

    event_notify(sizeof (struct event_mousepress), &mousepress);

}

void mouse_notifymouserelease(struct mouse_interface *interface, unsigned int button)
{

    mouserelease.button = button;

    event_notify(sizeof (struct event_mouserelease), &mouserelease);

}

void mouse_notifymousemove(struct mouse_interface *interface, char relx, char rely)
{

    mousemove.relx = relx;
    mousemove.rely = rely;

    event_notify(sizeof (struct event_mousemove), &mousemove);

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

void module_init(void)
{

    mousepress.header.type = EVENT_MOUSEPRESS;
    mousepress.header.count = sizeof (struct event_mousepress) - sizeof (struct event_header);
    mouserelease.header.type = EVENT_MOUSERELEASE;
    mouserelease.header.count = sizeof (struct event_mouserelease) - sizeof (struct event_header);
    mousemove.header.type = EVENT_MOUSEMOVE;
    mousemove.header.count = sizeof (struct event_mousemove) - sizeof (struct event_header);

}

