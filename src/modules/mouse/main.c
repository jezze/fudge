#include <fudge.h>
#include <kernel.h>
#include <event/event.h>
#include <modules/system/system.h>
#include <modules/event/event.h>
#include "mouse.h"

static struct system_node root;
static struct system_node event;

void mouse_notify(struct mouse_interface *interface, void *buffer, unsigned int count)
{

    kernel_multicast(&interface->data.states, buffer, count);

}

void mouse_notifymove(struct mouse_interface *interface, char relx, char rely)
{

    struct {struct event_header header; struct event_mousemove mousemove;} message;

    message.header.type = EVENT_MOUSEMOVE;
    message.header.source = EVENT_ADDR_BROADCAST;
    message.header.destination = EVENT_ADDR_BROADCAST;
    message.header.length = sizeof (message);
    message.mousemove.relx = relx;
    message.mousemove.rely = rely;

    event_multicast(&event.states, &message.header);
    event_multicast(&interface->event.states, &message.header);

}

void mouse_notifypress(struct mouse_interface *interface, unsigned int button)
{

    struct {struct event_header header; struct event_mousepress mousepress;} message;

    message.header.type = EVENT_MOUSEPRESS;
    message.header.source = EVENT_ADDR_BROADCAST;
    message.header.destination = EVENT_ADDR_BROADCAST;
    message.header.length = sizeof (message);
    message.mousepress.button = button;

    event_multicast(&event.states, &message.header);
    event_multicast(&interface->event.states, &message.header);

}

void mouse_notifyrelease(struct mouse_interface *interface, unsigned int button)
{

    struct {struct event_header header; struct event_mouserelease mouserelease;} message;

    message.header.type = EVENT_MOUSERELEASE;
    message.header.source = EVENT_ADDR_BROADCAST;
    message.header.destination = EVENT_ADDR_BROADCAST;
    message.header.length = sizeof (message);
    message.mouserelease.button = button;

    event_multicast(&event.states, &message.header);
    event_multicast(&interface->event.states, &message.header);

}

void mouse_registerinterface(struct mouse_interface *interface, unsigned int id)
{

    resource_register(&interface->resource);
    system_addchild(&interface->root, &interface->data);
    system_addchild(&interface->root, &interface->event);
    system_addchild(&root, &interface->root);

    interface->id = id;

}

void mouse_unregisterinterface(struct mouse_interface *interface)
{

    resource_unregister(&interface->resource);
    system_removechild(&interface->root, &interface->data);
    system_removechild(&interface->root, &interface->event);
    system_removechild(&root, &interface->root);

}

void mouse_initinterface(struct mouse_interface *interface)
{

    resource_init(&interface->resource, RESOURCE_MOUSEINTERFACE, interface);
    system_initresourcenode(&interface->root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "if", &interface->resource);
    system_initresourcenode(&interface->data, SYSTEM_NODETYPE_MAILBOX, "data", &interface->resource);
    system_initresourcenode(&interface->event, SYSTEM_NODETYPE_MAILBOX, "event", &interface->resource);

}

void module_init(void)
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "mouse");
    system_initnode(&event, SYSTEM_NODETYPE_MAILBOX, "event");

}

void module_register(void)
{

    system_registernode(&root);
    system_addchild(&root, &event);

}

void module_unregister(void)
{

    system_unregisternode(&root);
    system_removechild(&root, &event);

}

