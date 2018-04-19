#include <fudge.h>
#include <kernel.h>
#include <event/event.h>
#include <modules/system/system.h>
#include <modules/event/event.h>
#include "mouse.h"

static struct system_node root;
static struct system_node event;
static struct list eventstates;

void mouse_notify(struct mouse_interface *interface, void *buffer, unsigned int count)
{

    kernel_multicast(&interface->datastates, buffer, count);

}

void mouse_notifymove(struct mouse_interface *interface, char relx, char rely)
{

    struct {struct event_header header; struct event_mousemove mousemove;} message;

    message.header.type = EVENT_MOUSEMOVE;
    message.header.source = EVENT_ADDR_BROADCAST;
    message.header.destination = EVENT_ADDR_BROADCAST;
    message.mousemove.relx = relx;
    message.mousemove.rely = rely;

    event_multicast(&eventstates, &message.header, sizeof (struct event_header) + sizeof (struct event_mousemove));
    event_multicast(&interface->eventstates, &message.header, sizeof (struct event_header) + sizeof (struct event_mousemove));

}

void mouse_notifypress(struct mouse_interface *interface, unsigned int button)
{

    struct {struct event_header header; struct event_mousepress mousepress;} message;

    message.header.type = EVENT_MOUSEPRESS;
    message.header.source = EVENT_ADDR_BROADCAST;
    message.header.destination = EVENT_ADDR_BROADCAST;
    message.mousepress.button = button;

    event_multicast(&eventstates, &message.header, sizeof (struct event_header) + sizeof (struct event_mousepress));
    event_multicast(&interface->eventstates, &message.header, sizeof (struct event_header) + sizeof (struct event_mousepress));

}

void mouse_notifyrelease(struct mouse_interface *interface, unsigned int button)
{

    struct {struct event_header header; struct event_mouserelease mouserelease;} message;

    message.header.type = EVENT_MOUSERELEASE;
    message.header.source = EVENT_ADDR_BROADCAST;
    message.header.destination = EVENT_ADDR_BROADCAST;
    message.mouserelease.button = button;

    event_multicast(&interface->eventstates, &message.header, sizeof (struct event_header) + sizeof (struct event_mouserelease));

}

static struct system_node *event_open(struct system_node *self, struct service_state *state)
{

    list_add(&eventstates, &state->item);

    return self;

}

static struct system_node *event_close(struct system_node *self, struct service_state *state)
{

    list_remove(&eventstates, &state->item);

    return self;

}

static struct system_node *interfacedata_open(struct system_node *self, struct service_state *state)
{

    struct mouse_interface *interface = self->resource->data;

    list_add(&interface->datastates, &state->item);

    return self;

}

static struct system_node *interfacedata_close(struct system_node *self, struct service_state *state)
{

    struct mouse_interface *interface = self->resource->data;

    list_remove(&interface->datastates, &state->item);

    return self;

}

static struct system_node *interfaceevent_open(struct system_node *self, struct service_state *state)
{

    struct mouse_interface *interface = self->resource->data;

    list_add(&interface->eventstates, &state->item);

    return self;

}

static struct system_node *interfaceevent_close(struct system_node *self, struct service_state *state)
{

    struct mouse_interface *interface = self->resource->data;

    list_remove(&interface->eventstates, &state->item);

    return self;

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
    system_initresourcenode(&interface->event, SYSTEM_NODETYPE_NORMAL, "event", &interface->resource);

    interface->data.open = interfacedata_open;
    interface->data.close = interfacedata_close;
    interface->event.open = interfaceevent_open;
    interface->event.close = interfaceevent_close;

}

void module_init(void)
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "mouse");
    system_initnode(&event, SYSTEM_NODETYPE_MAILBOX, "event");

    event.open = event_open;
    event.close = event_close;

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

