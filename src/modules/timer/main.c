#include <fudge.h>
#include <kernel.h>
#include "timer.h"

static unsigned int place1(void *interface, unsigned int id, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    switch (event)
    {

    case EVENT_LINK:
        return kernel_link(id, source);

    case EVENT_UNLINK:
        return kernel_unlink(id, source);

    }

    return MESSAGE_UNIMPLEMENTED;

}

static unsigned int place10(void *interface, unsigned int id, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    switch (event)
    {

    case EVENT_LINK:
        return kernel_link(id, source);

    case EVENT_UNLINK:
        return kernel_unlink(id, source);

    }

    return MESSAGE_UNIMPLEMENTED;

}

static unsigned int place100(void *interface, unsigned int id, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    switch (event)
    {

    case EVENT_LINK:
        return kernel_link(id, source);

    case EVENT_UNLINK:
        return kernel_unlink(id, source);

    }

    return MESSAGE_UNIMPLEMENTED;

}

static unsigned int place1000(void *interface, unsigned int id, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    switch (event)
    {

    case EVENT_LINK:
        return kernel_link(id, source);

    case EVENT_UNLINK:
        return kernel_unlink(id, source);

    }

    return MESSAGE_UNIMPLEMENTED;

}

void timer_notifytick1(struct timer_interface *interface, unsigned int counter)
{

    struct event_timertick timertick;

    timertick.counter = counter;

    kernel_notify(interface->ichannel1, EVENT_TIMERTICK, sizeof (struct event_timertick), &timertick);

}

void timer_notifytick10(struct timer_interface *interface, unsigned int counter)
{

    struct event_timertick timertick;

    timertick.counter = counter;

    kernel_notify(interface->ichannel10, EVENT_TIMERTICK, sizeof (struct event_timertick), &timertick);

}

void timer_notifytick100(struct timer_interface *interface, unsigned int counter)
{

    struct event_timertick timertick;

    timertick.counter = counter;

    kernel_notify(interface->ichannel100, EVENT_TIMERTICK, sizeof (struct event_timertick), &timertick);

}

void timer_notifytick1000(struct timer_interface *interface, unsigned int counter)
{

    struct event_timertick timertick;

    timertick.counter = counter;

    kernel_notify(interface->ichannel1000, EVENT_TIMERTICK, sizeof (struct event_timertick), &timertick);

}

void timer_registerinterface(struct timer_interface *interface)
{

    resource_register(&interface->resource);
    kernel_announce(interface->ichannel1, interface, interface->ichannel1, place1);
    kernel_announce(interface->ichannel10, interface, interface->ichannel10, place10);
    kernel_announce(interface->ichannel100, interface, interface->ichannel100, place100);
    kernel_announce(interface->ichannel1000, interface, interface->ichannel1000, place1000);

}

void timer_unregisterinterface(struct timer_interface *interface)
{

    resource_unregister(&interface->resource);
    kernel_unannounce(interface->ichannel1);
    kernel_unannounce(interface->ichannel10);
    kernel_unannounce(interface->ichannel100);
    kernel_unannounce(interface->ichannel1000);

}

void timer_initinterface(struct timer_interface *interface, unsigned int id, unsigned int ichannel1, unsigned int ichannel10, unsigned int ichannel100, unsigned int ichannel1000)
{

    resource_init(&interface->resource, RESOURCE_TIMERINTERFACE, interface);

    interface->id = id;
    interface->ichannel1 = ichannel1;
    interface->ichannel10 = ichannel10;
    interface->ichannel100 = ichannel100;
    interface->ichannel1000 = ichannel1000;

}

