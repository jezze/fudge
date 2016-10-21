#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "timer.h"

static struct system_node root;

void timer_notify(struct timer_interface *interface, void *buffer, unsigned int count)
{

    system_multicast(&interface->sleeplinks, buffer, count);

}

static unsigned int interfacesleep_open(struct system_node *self, struct service_state *state)
{

    struct timer_interface *interface = self->resource->data;

    list_add(&interface->sleeplinks, &state->link);

    return state->id;

}

static unsigned int interfacesleep_close(struct system_node *self, struct service_state *state)
{

    struct timer_interface *interface = self->resource->data;

    list_remove(&interface->sleeplinks, &state->link);

    return state->id;

}

static unsigned int interfacesleep_read(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    return system_readlink(&state->link, buffer, count);

}

void timer_registerinterface(struct timer_interface *interface, unsigned int id)
{

    resource_register(&interface->resource);
    system_addchild(&interface->root, &interface->sleep);
    system_addchild(&root, &interface->root);

    interface->id = id;

}

void timer_unregisterinterface(struct timer_interface *interface)
{

    resource_unregister(&interface->resource);
    system_removechild(&interface->root, &interface->sleep);
    system_removechild(&root, &interface->root);

}

void timer_initinterface(struct timer_interface *interface)
{

    resource_init(&interface->resource, RESOURCE_TIMERINTERFACE, interface);
    system_initresourcenode(&interface->root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "if", &interface->resource);
    system_initresourcenode(&interface->sleep, SYSTEM_NODETYPE_MAILBOX, "sleep", &interface->resource);

    interface->sleep.open = interfacesleep_open;
    interface->sleep.close = interfacesleep_close;
    interface->sleep.read = interfacesleep_read;

}

void module_init(void)
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "timer");

}

void module_register(void)
{

    system_registernode(&root);

}

void module_unregister(void)
{

    system_unregisternode(&root);

}

