#include <fudge.h>
#include <kernel.h>
#include "timer.h"

static struct service service;

static unsigned int service_match(unsigned int count, char *name)
{

    if (count >= 2 && buffer_match(name, ":", 1))
    {

        struct resource *current = 0;
        unsigned int index = cstring_toint(name[1]);
        unsigned int channelnum = cstring_toint(name[3]);
        unsigned int i;

        for (i = 0; (current = resource_foreachtype(current, RESOURCE_TIMERINTERFACE)); i++)
        {

            struct timer_interface *interface = current->data;

            if (i == index)
            {

                switch (channelnum)
                {

                case 0:
                    return interface->ichannel1;

                case 1:
                    return interface->ichannel10;

                case 2:
                    return interface->ichannel100;

                case 3:
                    return interface->ichannel1000;

                }

                return interface->ichannel1;

            }

        }

    }

    return 0;

}

static unsigned int place(void *interface, unsigned int ichannel, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    switch (event)
    {

    case EVENT_LINK:
        return kernel_link(ichannel, source);

    case EVENT_UNLINK:
        return kernel_unlink(ichannel, source);

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

    interface->ichannel1 = kernel_announce(interface, place);
    interface->ichannel10 = kernel_announce(interface, place);
    interface->ichannel100 = kernel_announce(interface, place);
    interface->ichannel1000 = kernel_announce(interface, place);

}

void timer_unregisterinterface(struct timer_interface *interface)
{

    resource_unregister(&interface->resource);
    kernel_unannounce(interface->ichannel1);
    kernel_unannounce(interface->ichannel10);
    kernel_unannounce(interface->ichannel100);
    kernel_unannounce(interface->ichannel1000);

}

void timer_initinterface(struct timer_interface *interface, unsigned int id)
{

    resource_init(&interface->resource, RESOURCE_TIMERINTERFACE, interface);

    interface->id = id;

}

void module_init(void)
{

    service_init(&service, "timer", service_match);
    service_register(&service);

}

