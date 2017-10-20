#include <fudge.h>
#include <kernel.h>
#include <event/event.h>
#include <modules/system/system.h>
#include <modules/event/event.h>
#include "mouse.h"

static struct system_node root;

void mouse_notify(struct mouse_interface *interface, void *buffer, unsigned int count)
{

    spinlock_acquire(&interface->datalock);
    kernel_multicast(&interface->datastates, buffer, count);
    spinlock_release(&interface->datalock);

}

void mouse_notifymove(struct mouse_interface *interface, char relx, char rely)
{

    struct {struct event_header header; struct event_mousemove mousemove;} message;

    message.header.type = EVENT_MOUSEMOVE;
    message.header.source = EVENT_ADDR_BROADCAST;
    message.header.destination = EVENT_ADDR_BROADCAST;
    message.mousemove.relx = relx;
    message.mousemove.rely = rely;

    spinlock_acquire(&interface->eventlock);
    event_multicast(&interface->eventstates, &message.header, sizeof (struct event_header) + sizeof (struct event_mousemove));
    spinlock_release(&interface->eventlock);

}

void mouse_notifypress(struct mouse_interface *interface, unsigned int button)
{

    struct {struct event_header header; struct event_mousepress mousepress;} message;

    message.header.type = EVENT_MOUSEPRESS;
    message.header.source = EVENT_ADDR_BROADCAST;
    message.header.destination = EVENT_ADDR_BROADCAST;
    message.mousepress.button = button;

    spinlock_acquire(&interface->eventlock);
    event_multicast(&interface->eventstates, &message.header, sizeof (struct event_header) + sizeof (struct event_mousepress));
    spinlock_release(&interface->eventlock);

}

void mouse_notifyrelease(struct mouse_interface *interface, unsigned int button)
{

    struct {struct event_header header; struct event_mouserelease mouserelease;} message;

    message.header.type = EVENT_MOUSERELEASE;
    message.header.source = EVENT_ADDR_BROADCAST;
    message.header.destination = EVENT_ADDR_BROADCAST;
    message.mouserelease.button = button;

    spinlock_acquire(&interface->eventlock);
    event_multicast(&interface->eventstates, &message.header, sizeof (struct event_header) + sizeof (struct event_mouserelease));
    spinlock_release(&interface->eventlock);

}

static unsigned int interfacedata_open(struct system_node *self, struct service_state *state)
{

    struct mouse_interface *interface = self->resource->data;

    list_lockadd(&interface->datastates, &state->item, &interface->datalock);

    return (unsigned int)self;

}

static unsigned int interfacedata_close(struct system_node *self, struct service_state *state)
{

    struct mouse_interface *interface = self->resource->data;

    list_lockremove(&interface->datastates, &state->item, &interface->datalock);

    return (unsigned int)self;

}

static unsigned int interfaceevent_open(struct system_node *self, struct service_state *state)
{

    struct mouse_interface *interface = self->resource->data;

    list_lockadd(&interface->eventstates, &state->item, &interface->eventlock);

    return (unsigned int)self;

}

static unsigned int interfaceevent_close(struct system_node *self, struct service_state *state)
{

    struct mouse_interface *interface = self->resource->data;

    list_lockremove(&interface->eventstates, &state->item, &interface->eventlock);

    return (unsigned int)self;

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

}

void module_register(void)
{

    system_registernode(&root);

}

void module_unregister(void)
{

    system_unregisternode(&root);

}

