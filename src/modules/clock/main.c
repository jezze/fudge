#include <fudge.h>
#include <kernel.h>
#include "clock.h"

static unsigned int place(unsigned int id, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    struct clock_interface *interface = (struct clock_interface *)id;

    switch (event)
    {

    case EVENT_INFO:
        return interface->getinfo(source);

    }

    return 0;

}

void clock_registerinterface(struct clock_interface *interface)
{

    resource_register(&interface->resource);
    kernel_announce(interface->ichannel, (unsigned int)interface, place);

}

void clock_unregisterinterface(struct clock_interface *interface)
{

    resource_unregister(&interface->resource);

}

void clock_initinterface(struct clock_interface *interface, unsigned int id, unsigned int ichannel, unsigned int (*getinfo)(unsigned int source))
{

    resource_init(&interface->resource, RESOURCE_CLOCKINTERFACE, interface);

    interface->id = id;
    interface->ichannel = ichannel;
    interface->getinfo = getinfo;

}

