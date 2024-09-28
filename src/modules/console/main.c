#include <fudge.h>
#include <kernel.h>
#include "console.h"

static struct service service;

static unsigned int service_match(unsigned int count, char *name)
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
                return interface->ichannel;

        }

    }

    return 0;

}

static unsigned int ondata(struct console_interface *interface, void *data, unsigned int count)
{

    return interface->ondata(data, count);

}

static unsigned int place(void *interface, unsigned int ichannel, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    switch (event)
    {

    case EVENT_DATA:
        return ondata(interface, data, count);

    case EVENT_LINK:
        return kernel_link(ichannel, source);

    case EVENT_UNLINK:
        return kernel_unlink(ichannel, source);

    }

    return MESSAGE_UNIMPLEMENTED;

}

void console_notifydata(struct console_interface *interface, unsigned char data)
{

    struct event_consoledata consoledata;

    consoledata.data = data;

    kernel_notify(interface->ichannel, EVENT_CONSOLEDATA, sizeof (struct event_consoledata), &consoledata);

}

void console_registerinterface(struct console_interface *interface)
{

    resource_register(&interface->resource);

    interface->ichannel = kernel_announce(interface->ichannel, interface, place);

}

void console_unregisterinterface(struct console_interface *interface)
{

    resource_unregister(&interface->resource);
    kernel_unannounce(interface->ichannel);

}

void console_initinterface(struct console_interface *interface, unsigned int id, unsigned int ichannel, unsigned int (*ondata)(void *buffer, unsigned int count))
{

    resource_init(&interface->resource, RESOURCE_CONSOLEINTERFACE, interface);

    interface->id = id;
    interface->ichannel = ichannel;
    interface->ondata = ondata;

}

void module_init(void)
{

    service_init(&service, "console", service_match);
    resource_register(&service.resource);

}

