#include <fudge.h>
#include <kernel.h>
#include "console.h"

static struct service service;

static struct node *service_match(unsigned int count, char *name)
{

    if (count == 2 && buffer_match(name, ":", 1))
    {

        struct resource *current = 0;
        unsigned int index = cstring_toint(name[1]);
        unsigned int i;

        for (i = 0; (current = resource_foreachtype(current, RESOURCE_CONSOLEINTERFACE)); i++)
        {

            struct console_interface *interface = current->data;

            if (i == index)
                return kernel_getnode(&interface->resource.sources, 0);

        }

    }

    return 0;

}

static unsigned int ondata(struct console_interface *interface, struct node *source, void *data, unsigned int count)
{

    return interface->ondata(source, data, count);

}

static unsigned int place(struct node *source, struct node *target, unsigned int event, unsigned int count, void *data)
{

    switch (event)
    {

    case EVENT_DATA:
        return ondata(target->resource->data, source, data, count);

    }

    return MESSAGE_UNIMPLEMENTED;

}

void console_notifydata(struct console_interface *interface, unsigned char data)
{

    struct event_consoledata consoledata;

    consoledata.data = data;

    kernel_notify(0, &interface->resource.sources, &interface->resource.targets, EVENT_CONSOLEDATA, sizeof (struct event_consoledata), &consoledata);

}

void console_registerinterface(struct console_interface *interface)
{

    resource_register(&interface->resource);

}

void console_unregisterinterface(struct console_interface *interface)
{

    resource_unregister(&interface->resource);

}

void console_initinterface(struct console_interface *interface, unsigned int id, unsigned int (*ondata)(struct node *source, void *buffer, unsigned int count))
{

    resource_init(&interface->resource, RESOURCE_CONSOLEINTERFACE, interface);
    kernel_link(&interface->resource.sources, 0, &interface->resource, place);

    interface->id = id;
    interface->ondata = ondata;

}

void module_init(void)
{

    service_init(&service, "console", service_match);
    service_register(&service);

}

