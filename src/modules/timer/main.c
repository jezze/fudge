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

static unsigned int service_place(unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    switch (event)
    {

    case EVENT_LINK:
        return kernel_linknode(target, source);

    case EVENT_UNLINK:
        return kernel_unlinknode(target, source);

    }

    return MESSAGE_UNIMPLEMENTED;

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
    interface->inodes[0] = kernel_addnode("timer0", &interface->resource, &service);
    interface->inodes[1] = kernel_addnode("timer1", &interface->resource, &service);
    interface->inodes[2] = kernel_addnode("timer2", &interface->resource, &service);
    interface->inodes[3] = kernel_addnode("timer3", &interface->resource, &service);

}

void module_init(void)
{

    service_init(&service, "timer", service_foreach, service_getinode, 0, service_place);
    service_register(&service);

}

