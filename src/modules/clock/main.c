#include <fudge.h>
#include <kernel.h>
#include "clock.h"

static struct service service;

static unsigned int service_match(unsigned int count, char *name)
{

    if (count == 2 && buffer_match(name, ":", 1))
    {

        struct resource *current = 0;
        unsigned int index = cstring_toint(name[1]);
        unsigned int i;

        for (i = 0; (current = resource_foreachtype(current, RESOURCE_CLOCKINTERFACE)); i++)
        {

            if (i == index)
                return kernel_encodenodelist(&current->sources, 0);

        }

    }

    return 0;

}

static unsigned int oninfo(struct clock_interface *interface, unsigned int source)
{

    return interface->oninfo(source);

}

static unsigned int place(unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    struct clock_interface *interface = kernel_getinterface(target);

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

}

void clock_unregisterinterface(struct clock_interface *interface)
{

    resource_unregister(&interface->resource);

}

void clock_initinterface(struct clock_interface *interface, unsigned int id, unsigned int (*oninfo)(unsigned int source))
{

    resource_init(&interface->resource, RESOURCE_CLOCKINTERFACE, interface);
    kernel_link(&interface->resource.sources, 0, &interface->resource, place);

    interface->id = id;
    interface->oninfo = oninfo;

}

void module_init(void)
{

    service_init(&service, "clock", service_match);
    service_register(&service);

}

