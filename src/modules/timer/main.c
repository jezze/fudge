#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "timer.h"

static struct system_node root;
static unsigned int jiffies;

void timer_notifytick(struct timer_interface *interface)
{

    struct event_timertick timertick;

    timertick.counter = jiffies++;

    kernel_notify(&interface->event1.links, EVENT_TIMERTICK, sizeof (struct event_timertick), &timertick);

    if (!(jiffies % 10))
        kernel_notify(&interface->event10.links, EVENT_TIMERTICK, sizeof (struct event_timertick), &timertick);

    if (!(jiffies % 100))
        kernel_notify(&interface->event100.links, EVENT_TIMERTICK, sizeof (struct event_timertick), &timertick);

    if (!(jiffies % 1000))
        kernel_notify(&interface->event1000.links, EVENT_TIMERTICK, sizeof (struct event_timertick), &timertick);

}

void timer_registerinterface(struct timer_interface *interface)
{

    resource_register(&interface->resource);
    system_addchild(&interface->root, &interface->ctrl);
    system_addchild(&interface->root, &interface->event1);
    system_addchild(&interface->root, &interface->event10);
    system_addchild(&interface->root, &interface->event100);
    system_addchild(&interface->root, &interface->event1000);
    system_addchild(&root, &interface->root);

}

void timer_unregisterinterface(struct timer_interface *interface)
{

    resource_unregister(&interface->resource);
    system_removechild(&interface->root, &interface->ctrl);
    system_removechild(&interface->root, &interface->event1);
    system_removechild(&interface->root, &interface->event10);
    system_removechild(&interface->root, &interface->event100);
    system_removechild(&interface->root, &interface->event1000);
    system_removechild(&root, &interface->root);

}

void timer_initinterface(struct timer_interface *interface, unsigned int id)
{

    resource_init(&interface->resource, RESOURCE_TIMERINTERFACE, interface);
    system_initnode(&interface->root, SYSTEM_NODETYPE_MULTIGROUP, "if");
    system_initnode(&interface->ctrl, SYSTEM_NODETYPE_NORMAL, "ctrl");
    system_initnode(&interface->event1, SYSTEM_NODETYPE_NORMAL, "event1");
    system_initnode(&interface->event10, SYSTEM_NODETYPE_NORMAL, "event10");
    system_initnode(&interface->event100, SYSTEM_NODETYPE_NORMAL, "event100");
    system_initnode(&interface->event1000, SYSTEM_NODETYPE_NORMAL, "event1000");

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

