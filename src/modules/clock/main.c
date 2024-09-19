#include <fudge.h>
#include <kernel.h>
#include "clock.h"

static unsigned int oninfo(struct clock_interface *interface, unsigned int source)
{

    return interface->oninfo(source);

}

static unsigned int place(void *interface, unsigned int ichannel, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    switch (event)
    {

    case EVENT_INFO:
        return oninfo(interface, source);

    }

    return MESSAGE_UNIMPLEMENTED;

}

void clock_registerinterface(struct clock_interface *interface)
{

    resource_register(&interface->resource);
    kernel_announce(interface->ichannel, interface, 0, place);

}

void clock_unregisterinterface(struct clock_interface *interface)
{

    resource_unregister(&interface->resource);
    kernel_unannounce(interface->ichannel);

}

void clock_initinterface(struct clock_interface *interface, unsigned int id, unsigned int ichannel, unsigned int (*oninfo)(unsigned int source))
{

    resource_init(&interface->resource, RESOURCE_CLOCKINTERFACE, interface);

    interface->id = id;
    interface->ichannel = ichannel;
    interface->oninfo = oninfo;

}

