#include <fudge.h>
#include <net.h>
#include <kernel.h>
#include "ethernet.h"

static unsigned int place(unsigned int id, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    struct ethernet_interface *interface = (struct ethernet_interface *)id;

    switch (event)
    {

    case EVENT_DATA:
        return interface->ondata(data, count);

    case EVENT_INFO:
        return interface->oninfo(source);

    case EVENT_LINK:
        return kernel_link(interface->ichannel, source, interface->ichannel);

    case EVENT_UNLINK:
        return kernel_unlink(interface->ichannel, source);

    }

    return EVENT_UNIMPLEMENTED;

}

void ethernet_notify(struct ethernet_interface *interface, void *buffer, unsigned int count)
{

    kernel_notify(interface->ichannel, EVENT_DATA, count, buffer);

}

void ethernet_registerinterface(struct ethernet_interface *interface)
{

    resource_register(&interface->resource);
    kernel_announce(interface->ichannel, (unsigned int)interface, place);

}

void ethernet_unregisterinterface(struct ethernet_interface *interface)
{

    resource_unregister(&interface->resource);

}

void ethernet_initinterface(struct ethernet_interface *interface, unsigned int id, unsigned int ichannel, unsigned int (*oninfo)(unsigned int source), unsigned int (*ondata)(void *buffer, unsigned int count))
{

    resource_init(&interface->resource, RESOURCE_ETHERNETINTERFACE, interface);

    interface->id = id;
    interface->ichannel = ichannel;
    interface->oninfo = oninfo;
    interface->ondata = ondata;

}

