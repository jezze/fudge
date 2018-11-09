#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "timer.h"

static struct system_node root;

void timer_notify(struct timer_interface *interface, void *buffer, unsigned int count)
{

    union event_message message;

    event_createdata(&message.header, EVENT_BROADCAST, 0);
    event_appenddata(&message.header, count, buffer);
    kernel_multicast(&interface->data.states, &message.header, message.header.length);

}

void timer_notifytick(struct timer_interface *interface, unsigned int counter)
{

    union event_message message;

    event_createtimertick(&message.header, counter);
    kernel_multicast(&interface->event.states, &message.header, message.header.length);

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

