#include <fudge.h>
#include <net.h>
#include <kernel.h>
#include "ethernet.h"

static unsigned int ondata(struct ethernet_interface *interface, void *data, unsigned int count)
{

    return interface->ondata(data, count);

}

static unsigned int oninfo(struct ethernet_interface *interface, unsigned int source)
{

    return interface->oninfo(source);

}

static unsigned int place(void *interface, unsigned int id, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    switch (event)
    {

    case EVENT_DATA:
        return ondata(interface, data, count);

    case EVENT_INFO:
        return oninfo(interface, source);

    case EVENT_LINK:
        return kernel_link(id, source);

    case EVENT_UNLINK:
        return kernel_unlink(id, source);

    }

    return MESSAGE_UNIMPLEMENTED;

}

void ethernet_notifydata(struct ethernet_interface *interface, void *buffer, unsigned int count)
{

    kernel_notify(interface->ichannel, EVENT_DATA, count, buffer);

}

void ethernet_registerinterface(struct ethernet_interface *interface)
{

    resource_register(&interface->resource);
    kernel_announce(interface->ichannel, interface, interface->ichannel, place);

}

void ethernet_unregisterinterface(struct ethernet_interface *interface)
{

    resource_unregister(&interface->resource);
    kernel_unannounce(interface->ichannel);

}

void ethernet_initinterface(struct ethernet_interface *interface, unsigned int id, unsigned int ichannel, unsigned int (*oninfo)(unsigned int source), unsigned int (*ondata)(void *buffer, unsigned int count))
{

    resource_init(&interface->resource, RESOURCE_ETHERNETINTERFACE, interface);

    interface->id = id;
    interface->ichannel = ichannel;
    interface->oninfo = oninfo;
    interface->ondata = ondata;

}

