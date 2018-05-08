#include <fudge.h>
#include <kernel.h>
#include <event/event.h>
#include <modules/system/system.h>
#include <modules/event/event.h>
#include "timer.h"

static struct system_node root;

void timer_notify(struct timer_interface *interface, void *buffer, unsigned int count)
{

    kernel_multicast(&interface->sleep.states, buffer, count);

}

void timer_notifytick(struct timer_interface *interface, unsigned int counter)
{

    struct {struct event_header header; struct event_timertick timertick;} message;

    message.header.type = EVENT_TIMERTICK;
    message.header.source = EVENT_ADDR_BROADCAST;
    message.header.destination = EVENT_ADDR_BROADCAST;
    message.header.length = sizeof (message);
    message.timertick.counter = counter;

    event_multicast(&interface->event.states, &message.header);

}

void timer_registerinterface(struct timer_interface *interface, unsigned int id)
{

    resource_register(&interface->resource);
    system_addchild(&interface->root, &interface->sleep);
    system_addchild(&interface->root, &interface->event);
    system_addchild(&root, &interface->root);

    interface->id = id;

}

void timer_unregisterinterface(struct timer_interface *interface)
{

    resource_unregister(&interface->resource);
    system_removechild(&interface->root, &interface->sleep);
    system_removechild(&interface->root, &interface->event);
    system_removechild(&root, &interface->root);

}

void timer_initinterface(struct timer_interface *interface)
{

    resource_init(&interface->resource, RESOURCE_TIMERINTERFACE, interface);
    system_initresourcenode(&interface->root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "if", &interface->resource);
    system_initresourcenode(&interface->sleep, SYSTEM_NODETYPE_MAILBOX, "sleep", &interface->resource);
    system_initresourcenode(&interface->event, SYSTEM_NODETYPE_MAILBOX, "event", &interface->resource);

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

