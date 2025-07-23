#include <fudge.h>
#include <kernel.h>
#include "clock.h"

static struct service service;

static unsigned int oninfo(struct clock_interface *interface, unsigned int source)
{

    return interface->oninfo(source);

}

static unsigned int service_place(unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    struct clock_interface *interface = kernel_getnodeinterface(target);

    switch (event)
    {

    case EVENT_LINK:
        return kernel_linknode(target, source);

    case EVENT_UNLINK:
        return kernel_unlinknode(target, source);

    case EVENT_INFO:
        return oninfo(interface, source);

    }

    return MESSAGE_UNIMPLEMENTED;

}

void clock_registerinterface(struct clock_interface *interface)
{

    resource_register(&interface->resource);

}

void clock_unregisterinterface(struct clock_interface *interface)
{

    resource_unregister(&interface->resource);

}

void clock_initinterface(struct clock_interface *interface, unsigned int id, unsigned int (*oninfo)(unsigned int source))
{

    resource_init(&interface->resource, RESOURCE_CLOCKINTERFACE, interface);

    interface->id = id;
    interface->inode = kernel_addnode("clock", &interface->resource, &service);
    interface->oninfo = oninfo;

}

void module_init(void)
{

    service_init(&service, 0, service_place);
    service_register(&service);

}

