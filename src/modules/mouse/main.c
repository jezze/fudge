#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "mouse.h"

static struct system_node root;
static struct system_node event;

void mouse_notify(struct mouse_interface *interface, void *buffer, unsigned int count)
{

    kernel_multicastdata(&interface->data.states, buffer, count);

}

void mouse_notifymove(struct mouse_interface *interface, char relx, char rely)
{

    struct {struct event_header header; struct event_mousemove mousemove;} message;

    message.mousemove.relx = relx;
    message.mousemove.rely = rely;

    event_initheader(&message.header, EVENT_MOUSEMOVE, EVENT_ADDR_SELF, EVENT_ADDR_BROADCAST, sizeof (struct event_mousemove));
    kernel_multicast(&event.states, &message.header);
    kernel_multicast(&interface->event.states, &message.header);

}

void mouse_notifypress(struct mouse_interface *interface, unsigned int button)
{

    struct {struct event_header header; struct event_mousepress mousepress;} message;

    message.mousepress.button = button;

    event_initheader(&message.header, EVENT_MOUSEPRESS, EVENT_ADDR_SELF, EVENT_ADDR_BROADCAST, sizeof (struct event_mousepress));
    kernel_multicast(&event.states, &message.header);
    kernel_multicast(&interface->event.states, &message.header);

}

void mouse_notifyrelease(struct mouse_interface *interface, unsigned int button)
{

    struct {struct event_header header; struct event_mouserelease mouserelease;} message;

    message.mouserelease.button = button;

    event_initheader(&message.header, EVENT_MOUSERELEASE, EVENT_ADDR_SELF, EVENT_ADDR_BROADCAST, sizeof (struct event_mouserelease));
    kernel_multicast(&event.states, &message.header);
    kernel_multicast(&interface->event.states, &message.header);

}

void mouse_registerinterface(struct mouse_interface *interface)
{

    resource_register(&interface->resource);
    system_addchild(&interface->root, &interface->data);
    system_addchild(&interface->root, &interface->event);
    system_addchild(&root, &interface->root);

}

void mouse_unregisterinterface(struct mouse_interface *interface)
{

    resource_unregister(&interface->resource);
    system_removechild(&interface->root, &interface->data);
    system_removechild(&interface->root, &interface->event);
    system_removechild(&root, &interface->root);

}

void mouse_initinterface(struct mouse_interface *interface, unsigned int id)
{

    resource_init(&interface->resource, RESOURCE_MOUSEINTERFACE, interface);
    system_initnode(&interface->root, SYSTEM_NODETYPE_MULTIGROUP, "if");
    system_initnode(&interface->data, SYSTEM_NODETYPE_MAILBOX, "data");
    system_initnode(&interface->event, SYSTEM_NODETYPE_MAILBOX, "event");

    interface->id = id;

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

