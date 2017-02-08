#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/event/event.h>
#include "keyboard.h"

static struct system_node root;
static struct system_node event;
static struct list eventlinks;

void keyboard_notify(struct keyboard_interface *interface, void *buffer, unsigned int count)
{

    kernel_multicast(&interface->datalinks, buffer, count);

}

void keyboard_notifypress(unsigned char scancode)
{

    struct {struct event_header header; struct event_keypress keypress;} message;

    message.header.type = EVENT_KEYPRESS;
    message.header.source = 0;
    message.header.destination = 0;
    message.keypress.scancode = scancode;

    event_multicast(&eventlinks, &message.header, sizeof (struct event_header) + sizeof (struct event_keypress));

}

void keyboard_notifyrelease(unsigned char scancode)
{

    struct {struct event_header header; struct event_keyrelease keyrelease;} message;

    message.header.type = EVENT_KEYRELEASE;
    message.header.source = 0;
    message.header.destination = 0;
    message.keyrelease.scancode = scancode;

    event_multicast(&eventlinks, &message.header, sizeof (struct event_header) + sizeof (struct event_keyrelease));

}

static unsigned int event_open(struct system_node *self, struct service_state *state)
{

    list_add(&eventlinks, &state->link);

    return state->id;

}

static unsigned int event_close(struct system_node *self, struct service_state *state)
{

    list_remove(&eventlinks, &state->link);

    return state->id;

}

static unsigned int event_read(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    return task_read(state->link.data, buffer, count);

}

static unsigned int event_write(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    return event_send(&eventlinks, state, buffer, count);

}

static unsigned int interfacedata_open(struct system_node *self, struct service_state *state)
{

    struct keyboard_interface *interface = self->resource->data;

    list_add(&interface->datalinks, &state->link);

    return state->id;

}

static unsigned int interfacedata_close(struct system_node *self, struct service_state *state)
{

    struct keyboard_interface *interface = self->resource->data;

    list_remove(&interface->datalinks, &state->link);

    return state->id;

}

static unsigned int interfacedata_read(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    return task_read(state->link.data, buffer, count);

}

void keyboard_registerinterface(struct keyboard_interface *interface, unsigned int id)
{

    resource_register(&interface->resource);
    system_addchild(&interface->root, &interface->data);
    system_addchild(&root, &interface->root);

    interface->id = id;

}

void keyboard_unregisterinterface(struct keyboard_interface *interface)
{

    resource_unregister(&interface->resource);
    system_removechild(&interface->root, &interface->data);
    system_removechild(&root, &interface->root);

}

void keyboard_initinterface(struct keyboard_interface *interface)
{

    resource_init(&interface->resource, RESOURCE_KEYBOARDINTERFACE, interface);
    system_initresourcenode(&interface->root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "if", &interface->resource);
    system_initresourcenode(&interface->data, SYSTEM_NODETYPE_NORMAL, "data", &interface->resource);

    interface->data.open = interfacedata_open;
    interface->data.close = interfacedata_close;
    interface->data.read = interfacedata_read;

}

void module_init(void)
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "keyboard");
    system_initnode(&event, SYSTEM_NODETYPE_NORMAL, "event");

    event.open = event_open;
    event.close = event_close;
    event.read = event_read;
    event.write = event_write;

    system_addchild(&root, &event);

}

void module_register(void)
{

    system_registernode(&root);

}

void module_unregister(void)
{

    system_unregisternode(&root);

}

