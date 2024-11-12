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

            if (i == index)
                return kernel_encodenodelist(&current->sources, 0);

        }

    }

    return 0;

}

static unsigned int ondata(struct ethernet_interface *interface, unsigned int source, void *data, unsigned int count)
{

    return interface->ondata(source, data, count);

}

static unsigned int oninfo(struct ethernet_interface *interface, unsigned int source)
{

    return interface->oninfo(source);

}

static unsigned int place(unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    struct ethernet_interface *interface = kernel_getinterface(target);

    switch (event)
    {

    case EVENT_DATA:
        return ondata(interface, source, data, count);

    case EVENT_INFO:
        return oninfo(interface, source);

    }

    return MESSAGE_UNIMPLEMENTED;

}

void ethernet_notifydata(struct ethernet_interface *interface, void *buffer, unsigned int count)
{

    kernel_notify(kernel_encodenodelist(&interface->resource.sources, 0), &interface->resource.targets, EVENT_DATA, count, buffer);

}

void ethernet_registerinterface(struct ethernet_interface *interface)
{

    resource_register(&interface->resource);

}

void ethernet_unregisterinterface(struct ethernet_interface *interface)
{

    resource_unregister(&interface->resource);

}

void ethernet_initinterface(struct ethernet_interface *interface, unsigned int id, unsigned int (*oninfo)(unsigned int source), unsigned int (*ondata)(unsigned int source, void *buffer, unsigned int count))
{

    resource_init(&interface->resource, RESOURCE_ETHERNETINTERFACE, interface);
    kernel_link(&interface->resource.sources, 0, &interface->resource, place);

    interface->id = id;
    interface->oninfo = oninfo;
    interface->ondata = ondata;

}

void module_init(void)
{

    service_init(&service, "ethernet", service_match);
    service_register(&service);

}

