#include <fudge.h>
#include <kernel.h>
#include "timer.h"

static struct service service;

static struct node *service_match(unsigned int count, char *name)
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
                    return kernel_getnode(&interface->resource.sources, 0);

                case 1:
                    return kernel_getnode(&interface->resource.sources, 1);

                case 2:
                    return kernel_getnode(&interface->resource.sources, 2);

                case 3:
                    return kernel_getnode(&interface->resource.sources, 3);

                }

                return kernel_getnode(&interface->resource.sources, 0);

            }

        }

    }

    return 0;

}

void timer_notifytick1(struct timer_interface *interface, unsigned int counter)
{

    struct event_timertick timertick;

    timertick.counter = counter;

    kernel_notify(0, &interface->resource.sources, &interface->resource.targets, EVENT_TIMERTICK, sizeof (struct event_timertick), &timertick);

}

void timer_notifytick10(struct timer_interface *interface, unsigned int counter)
{

    struct event_timertick timertick;

    timertick.counter = counter;

    kernel_notify(1, &interface->resource.sources, &interface->resource.targets, EVENT_TIMERTICK, sizeof (struct event_timertick), &timertick);

}

void timer_notifytick100(struct timer_interface *interface, unsigned int counter)
{

    struct event_timertick timertick;

    timertick.counter = counter;

    kernel_notify(2, &interface->resource.sources, &interface->resource.targets, EVENT_TIMERTICK, sizeof (struct event_timertick), &timertick);

}

void timer_notifytick1000(struct timer_interface *interface, unsigned int counter)
{

    struct event_timertick timertick;

    timertick.counter = counter;

    kernel_notify(3, &interface->resource.sources, &interface->resource.targets, EVENT_TIMERTICK, sizeof (struct event_timertick), &timertick);

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
    kernel_picknode(&interface->resource.sources, 0, &interface->resource, 0);
    kernel_picknode(&interface->resource.sources, 0, &interface->resource, 0);
    kernel_picknode(&interface->resource.sources, 0, &interface->resource, 0);
    kernel_picknode(&interface->resource.sources, 0, &interface->resource, 0);

    interface->id = id;

}

void module_init(void)
{

    service_init(&service, "timer", service_match);
    service_register(&service);

}

