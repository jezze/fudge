#include <fudge.h>
#include <net.h>
#include <kernel.h>
#include "ethernet.h"

static struct service service;

static unsigned int service_match(unsigned int count, char *name)
{

    if (count == 2 && buffer_match(name, ":", 1))
    {

        struct resource *current = 0;
        unsigned int index = cstring_toint(name[1]);
        unsigned int i;

        for (i = 0; (current = resource_foreachtype(current, RESOURCE_ETHERNETINTERFACE)); i++)
        {

            struct ethernet_interface *interface = current->data;

            if (i == index)
                return interface->ichannel;

        }

    }

    return 0;

}

static unsigned int ondata(struct ethernet_interface *interface, void *data, unsigned int count)
{

    return interface->ondata(data, count);

}

static unsigned int oninfo(struct ethernet_interface *interface, unsigned int source)
{

    return interface->oninfo(source);

}

static unsigned int place(void *interface, unsigned int ichannel, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    switch (event)
    {

    case EVENT_DATA:
        return ondata(interface, data, count);

    case EVENT_INFO:
        return oninfo(interface, source);

    case EVENT_LINK:
        return kernel_link(ichannel, source);

    case EVENT_UNLINK:
        return kernel_unlink(ichannel, source);

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

    interface->ichannel = kernel_announce(interface, place);

}

void ethernet_unregisterinterface(struct ethernet_interface *interface)
{

    resource_unregister(&interface->resource);
    kernel_unannounce(interface->ichannel);

}

void ethernet_initinterface(struct ethernet_interface *interface, unsigned int id, unsigned int (*oninfo)(unsigned int source), unsigned int (*ondata)(void *buffer, unsigned int count))
{

    resource_init(&interface->resource, RESOURCE_ETHERNETINTERFACE, interface);

    interface->id = id;
    interface->oninfo = oninfo;
    interface->ondata = ondata;

}

void module_init(void)
{

    service_init(&service, "ethernet", service_match);
    resource_register(&service.resource);

}

