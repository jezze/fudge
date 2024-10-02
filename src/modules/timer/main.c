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
                    return &interface->node1;

                case 1:
                    return &interface->node10;

                case 2:
                    return &interface->node100;

                case 3:
                    return &interface->node1000;

                }

                return &interface->node1;

            }

        }

    }

    return 0;

}

void timer_notifytick1(struct timer_interface *interface, unsigned int counter)
{

    struct event_timertick timertick;

    timertick.counter = counter;

    kernel_notify(&interface->node1, EVENT_TIMERTICK, sizeof (struct event_timertick), &timertick);

}

void timer_notifytick10(struct timer_interface *interface, unsigned int counter)
{

    struct event_timertick timertick;

    timertick.counter = counter;

    kernel_notify(&interface->node10, EVENT_TIMERTICK, sizeof (struct event_timertick), &timertick);

}

void timer_notifytick100(struct timer_interface *interface, unsigned int counter)
{

    struct event_timertick timertick;

    timertick.counter = counter;

    kernel_notify(&interface->node100, EVENT_TIMERTICK, sizeof (struct event_timertick), &timertick);

}

void timer_notifytick1000(struct timer_interface *interface, unsigned int counter)
{

    struct event_timertick timertick;

    timertick.counter = counter;

    kernel_notify(&interface->node1000, EVENT_TIMERTICK, sizeof (struct event_timertick), &timertick);

}

void timer_registerinterface(struct timer_interface *interface)
{

    resource_register(&interface->resource);
    kernel_announce(&interface->node1);
    kernel_announce(&interface->node10);
    kernel_announce(&interface->node100);
    kernel_announce(&interface->node1000);

}

void timer_unregisterinterface(struct timer_interface *interface)
{

    resource_unregister(&interface->resource);
    kernel_unannounce(&interface->node1);
    kernel_unannounce(&interface->node10);
    kernel_unannounce(&interface->node100);
    kernel_unannounce(&interface->node1000);

}

void timer_initinterface(struct timer_interface *interface, unsigned int id)
{

    resource_init(&interface->resource, RESOURCE_TIMERINTERFACE, interface);
    node_init(&interface->node1, interface, 0);
    node_init(&interface->node10, interface, 0);
    node_init(&interface->node100, interface, 0);
    node_init(&interface->node1000, interface, 0);

    interface->id = id;

}

void module_init(void)
{

    service_init(&service, "timer", service_match);
    service_register(&service);

}

