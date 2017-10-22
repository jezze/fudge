#include <fudge.h>
#include <kernel.h>
#include <event/event.h>
#include <modules/system/system.h>
#include <modules/event/event.h>
#include "keyboard.h"

static struct system_node root;

void keyboard_notify(struct keyboard_interface *interface, void *buffer, unsigned int count)
{

    kernel_multicast(&interface->datastates, buffer, count);

}

void keyboard_notifypress(struct keyboard_interface *interface, unsigned char scancode)
{

    struct {struct event_header header; struct event_keypress keypress;} message;

    message.header.type = EVENT_KEYPRESS;
    message.header.source = EVENT_ADDR_BROADCAST;
    message.header.destination = EVENT_ADDR_BROADCAST;
    message.keypress.scancode = scancode;

    event_multicast(&interface->eventstates, &message.header, sizeof (struct event_header) + sizeof (struct event_keypress));

}

void keyboard_notifyrelease(struct keyboard_interface *interface, unsigned char scancode)
{

    struct {struct event_header header; struct event_keyrelease keyrelease;} message;

    message.header.type = EVENT_KEYRELEASE;
    message.header.source = EVENT_ADDR_BROADCAST;
    message.header.destination = EVENT_ADDR_BROADCAST;
    message.keyrelease.scancode = scancode;

    event_multicast(&interface->eventstates, &message.header, sizeof (struct event_header) + sizeof (struct event_keyrelease));

}

static struct system_node *interfacedata_open(struct system_node *self, struct service_state *state)
{

    struct keyboard_interface *interface = self->resource->data;

    list_add(&interface->datastates, &state->item);

    return self;

}

static struct system_node *interfacedata_close(struct system_node *self, struct service_state *state)
{

    struct keyboard_interface *interface = self->resource->data;

    list_remove(&interface->datastates, &state->item);

    return self;

}

static struct system_node *interfaceevent_open(struct system_node *self, struct service_state *state)
{

    struct keyboard_interface *interface = self->resource->data;

    list_add(&interface->eventstates, &state->item);

    return self;

}

static struct system_node *interfaceevent_close(struct system_node *self, struct service_state *state)
{

    struct keyboard_interface *interface = self->resource->data;

    list_remove(&interface->eventstates, &state->item);

    return self;

}

void keyboard_registerinterface(struct keyboard_interface *interface, unsigned int id)
{

    resource_register(&interface->resource);
    system_addchild(&interface->root, &interface->data);
    system_addchild(&interface->root, &interface->event);
    system_addchild(&root, &interface->root);

    interface->id = id;

}

void keyboard_unregisterinterface(struct keyboard_interface *interface)
{

    resource_unregister(&interface->resource);
    system_removechild(&interface->root, &interface->data);
    system_removechild(&interface->root, &interface->event);
    system_removechild(&root, &interface->root);

}

void keyboard_initinterface(struct keyboard_interface *interface)
{

    resource_init(&interface->resource, RESOURCE_KEYBOARDINTERFACE, interface);
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

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "keyboard");

}

void module_register(void)
{

    system_registernode(&root);

}

void module_unregister(void)
{

    system_unregisternode(&root);

}

