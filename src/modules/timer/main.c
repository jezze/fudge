#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "timer.h"

static struct system_node root;

void timer_notify(struct timer_interface *interface, void *buffer, unsigned int count)
{

    kernel_notify(&interface->data.states, EVENT_DATA, buffer, count);

}

void timer_notifytick(struct timer_interface *interface, unsigned int counter)
{

    struct event_timertick timertick;

    timertick.counter = counter;

    kernel_notify(&interface->event.states, EVENT_TIMERTICK, &timertick, sizeof (struct event_timertick));

}

void timer_registerinterface(struct timer_interface *interface)
{

    resource_register(&interface->resource);
    system_addchild(&interface->root, &interface->data);
    system_addchild(&interface->root, &interface->event);
    system_addchild(&root, &interface->root);

}

void timer_unregisterinterface(struct timer_interface *interface)
{

    resource_unregister(&interface->resource);
    system_removechild(&interface->root, &interface->data);
    system_removechild(&interface->root, &interface->event);
    system_removechild(&root, &interface->root);

}

void timer_initinterface(struct timer_interface *interface, unsigned int id)
{

    resource_init(&interface->resource, RESOURCE_TIMERINTERFACE, interface);
    system_initnode(&interface->root, SYSTEM_NODETYPE_MULTIGROUP, "if");
    system_initnode(&interface->data, SYSTEM_NODETYPE_NORMAL, "data");
    system_initnode(&interface->event, SYSTEM_NODETYPE_NORMAL, "event");

    interface->id = id;

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

