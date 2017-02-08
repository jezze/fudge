#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/event/event.h>
#include "mouse.h"

static struct system_node root;
static struct system_node event;
static struct list eventlinks;

void mouse_notifymove(char relx, char rely)
{

    struct {struct event_header header; struct event_mousemove mousemove;} message;

    message.header.type = EVENT_MOUSEMOVE;
    message.header.source = 0;
    message.header.destination = 0;
    message.mousemove.relx = relx;
    message.mousemove.rely = rely;

    event_multicast(&eventlinks, &message.header, sizeof (struct event_header) + sizeof (struct event_mousemove));

}

void mouse_notifypress(unsigned int button)
{

    struct {struct event_header header; struct event_mousepress mousepress;} message;

    message.header.type = EVENT_MOUSEPRESS;
    message.header.source = 0;
    message.header.destination = 0;
    message.mousepress.button = button;

    event_multicast(&eventlinks, &message.header, sizeof (struct event_header) + sizeof (struct event_mousepress));

}

void mouse_notifyrelease(unsigned int button)
{

    struct {struct event_header header; struct event_mouserelease mouserelease;} message;

    message.header.type = EVENT_MOUSERELEASE;
    message.header.source = 0;
    message.header.destination = 0;
    message.mouserelease.button = button;

    event_multicast(&eventlinks, &message.header, sizeof (struct event_header) + sizeof (struct event_mouserelease));

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

    struct mouse_interface *interface = self->resource->data;

    list_add(&interface->datalinks, &state->link);

    return state->id;

}

static unsigned int interfacedata_close(struct system_node *self, struct service_state *state)
{

    struct mouse_interface *interface = self->resource->data;

    list_remove(&interface->datalinks, &state->link);

    return state->id;

}

static unsigned int interfacedata_read(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    return task_read(state->link.data, buffer, count);

}

void mouse_registerinterface(struct mouse_interface *interface, unsigned int id)
{

    resource_register(&interface->resource);
    system_addchild(&interface->root, &interface->data);
    system_addchild(&root, &interface->root);

    interface->id = id;

}

void mouse_unregisterinterface(struct mouse_interface *interface)
{

    resource_unregister(&interface->resource);
    system_removechild(&interface->root, &interface->data);
    system_removechild(&root, &interface->root);

}

void mouse_initinterface(struct mouse_interface *interface)
{

    resource_init(&interface->resource, RESOURCE_MOUSEINTERFACE, interface);
    system_initresourcenode(&interface->root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "if", &interface->resource);
    system_initresourcenode(&interface->data, SYSTEM_NODETYPE_NORMAL, "data", &interface->resource);

    interface->data.open = interfacedata_open;
    interface->data.close = interfacedata_close;
    interface->data.read = interfacedata_read;

}

void module_init(void)
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "mouse");
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

