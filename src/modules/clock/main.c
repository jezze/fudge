#include <fudge.h>
#include <kernel.h>
#include "clock.h"

static struct service service;

static struct node *service_match(unsigned int count, char *name)
{

    if (count == 2 && buffer_match(name, ":", 1))
    {

        struct resource *current = 0;
        unsigned int index = cstring_toint(name[1]);
        unsigned int i;

        for (i = 0; (current = resource_foreachtype(current, RESOURCE_CLOCKINTERFACE)); i++)
        {

            struct clock_interface *interface = current->data;

            if (i == index)
                return &interface->node;

        }

    }

    return 0;

}

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
    kernel_announce(&interface->node, interface, place);

}

void clock_unregisterinterface(struct clock_interface *interface)
{

    resource_unregister(&interface->resource);
    kernel_unannounce(&interface->node);

}

void clock_initinterface(struct clock_interface *interface, unsigned int id, unsigned int (*oninfo)(unsigned int source))
{

    resource_init(&interface->resource, RESOURCE_CLOCKINTERFACE, interface);
    node_init(&interface->node);

    interface->id = id;
    interface->oninfo = oninfo;

}

void module_init(void)
{

    service_init(&service, "clock", service_match);
    service_register(&service);

}

