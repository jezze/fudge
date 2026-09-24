#include <fudge.h>
#include <kernel.h>
#include "timer.h"

static struct node_operands operands;

static unsigned int operands_place(struct resource *resource, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    struct timer_interface *interface = resource->data;

    switch (event)
    {

    case EVENT_LINK:
        return kernel_linknode(&interface->service, source);

    case EVENT_UNLINK:
        return kernel_unlinknode(&interface->service, source);

    }

    return MESSAGE_FAILED;

}

void timer_notifytick1(struct timer_interface *interface, unsigned int counter)
{

    struct event_timertick timertick;

    timertick.counter = counter;

    kernel_notify(&interface->service, EVENT_TIMERTICK, sizeof (struct event_timertick), &timertick);

}

void timer_notifytick10(struct timer_interface *interface, unsigned int counter)
{

    /*
    struct event_timertick timertick;

    timertick.counter = counter;

    kernel_notify(&interface->service, EVENT_TIMERTICK, sizeof (struct event_timertick), &timertick);
    */

}

void timer_notifytick100(struct timer_interface *interface, unsigned int counter)
{

    /*
    struct event_timertick timertick;

    timertick.counter = counter;

    kernel_notify(&interface->service, EVENT_TIMERTICK, sizeof (struct event_timertick), &timertick);
    */

}

void timer_notifytick1000(struct timer_interface *interface, unsigned int counter)
{

    /*
    struct event_timertick timertick;

    timertick.counter = counter;

    kernel_notify(&interface->service, EVENT_TIMERTICK, sizeof (struct event_timertick), &timertick);
    */

}

void timer_registerinterface(struct timer_interface *interface)
{

    unsigned int inode = pool_picknode();

    if (inode)
    {

        struct node *node = pool_getnode(inode);

        node_reset(node, &interface->resource, &operands);

    }

    resource_register(&interface->resource);
    service_register(&interface->service, inode);

}

void timer_unregisterinterface(struct timer_interface *interface)
{

    resource_unregister(&interface->resource);
    service_unregister(&interface->service);

}

void timer_initinterface(struct timer_interface *interface, unsigned int id)
{

    resource_init(&interface->resource, RESOURCE_TIMERINTERFACE, interface);
    service_init(&interface->service, "timer");

    interface->id = id;

}

void module_init(void)
{

    node_operands_init(&operands, 0, operands_place);

}

