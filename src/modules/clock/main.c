#include <fudge.h>
#include <kernel.h>
#include "clock.h"

static struct node_operands operands;

static unsigned int oninfo(struct clock_interface *interface, unsigned int source)
{

    if (interface->oninfo)
    {

        struct event_clockinfo clockinfo;

        interface->oninfo(&clockinfo);

        return kernel_place(interface->service.inode, source, EVENT_CLOCKINFO, sizeof (struct event_clockinfo), &clockinfo);

    }

    return MESSAGE_FAILED;

}

static unsigned int operands_place(unsigned int target, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    struct resource *resource = pool_getnoderesource(target);

    if (resource)
    {

        struct clock_interface *interface = resource->data;

        switch (event)
        {

        case EVENT_LINK:
            return kernel_linknode(&interface->service.links, source);

        case EVENT_UNLINK:
            return kernel_unlinknode(&interface->service.links, source);

        case EVENT_INFO:
            return oninfo(interface, source);

        }

    }

    return MESSAGE_FAILED;

}

void clock_registerinterface(struct clock_interface *interface)
{

    unsigned int inode = pool_picknode(&interface->resource, &operands);

    resource_register(&interface->resource);
    service_register(&interface->service, inode);

}

void clock_unregisterinterface(struct clock_interface *interface)
{

    resource_unregister(&interface->resource);
    service_unregister(&interface->service);

}

void clock_initinterface(struct clock_interface *interface, unsigned int id, void (*oninfo)(struct event_clockinfo *clockinfo))
{

    resource_init(&interface->resource, RESOURCE_CLOCKINTERFACE, interface);
    service_init(&interface->service, "clock");

    interface->id = id;
    interface->oninfo = oninfo;

}

void module_init(void)
{

    node_operands_init(&operands, 0, operands_place);

}

