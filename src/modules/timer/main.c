#include <fudge.h>
#include <kernel.h>
#include "timer.h"

static struct service service;

static struct resource *service_foreach(struct resource *current)
{

    return resource_foreachtype(current, RESOURCE_TIMERINTERFACE);

}

static unsigned int service_getinode(struct resource *current, unsigned int index)
{

    struct timer_interface *interface = current->data;

    return interface->inodes[index];

}

void timer_notifytick1(struct timer_interface *interface, unsigned int counter)
{

    struct event_timertick timertick;

    timertick.counter = counter;

    kernel_notify(interface->inodes[0], EVENT_TIMERTICK, sizeof (struct event_timertick), &timertick);

}

void timer_notifytick10(struct timer_interface *interface, unsigned int counter)
{

    struct event_timertick timertick;

    timertick.counter = counter;

    kernel_notify(interface->inodes[1], EVENT_TIMERTICK, sizeof (struct event_timertick), &timertick);

}

void timer_notifytick100(struct timer_interface *interface, unsigned int counter)
{

    struct event_timertick timertick;

    timertick.counter = counter;

    kernel_notify(interface->inodes[2], EVENT_TIMERTICK, sizeof (struct event_timertick), &timertick);

}

void timer_notifytick1000(struct timer_interface *interface, unsigned int counter)
{

    struct event_timertick timertick;

    timertick.counter = counter;

    kernel_notify(interface->inodes[3], EVENT_TIMERTICK, sizeof (struct event_timertick), &timertick);

}

void timer_registerinterface(struct timer_interface *interface)
{

    resource_register(&interface->resource);

}

void timer_unregisterinterface(struct timer_interface *interface)
{

    resource_unregister(&interface->resource);

}

void timer_initinterface(struct timer_interface *interface, unsigned int id)
{

    resource_init(&interface->resource, RESOURCE_TIMERINTERFACE, interface);

    interface->id = id;
    interface->inodes[0] = kernel_link(0, 0, &interface->resource, 0);
    interface->inodes[1] = kernel_link(0, 0, &interface->resource, 0);
    interface->inodes[2] = kernel_link(0, 0, &interface->resource, 0);
    interface->inodes[3] = kernel_link(0, 0, &interface->resource, 0);

}

void module_init(void)
{

    service_init(&service, "timer", service_foreach, service_getinode);
    service_register(&service);

}

