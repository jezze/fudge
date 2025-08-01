#include <fudge.h>
#include <kernel.h>
#include "console.h"

static struct service service;

static unsigned int ondata(struct console_interface *interface, unsigned int source, void *data, unsigned int count)
{

    return interface->ondata(source, data, count);

}

static unsigned int service_place(unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    struct console_interface *interface = kernel_getnodeinterface(target);

    switch (event)
    {

    case EVENT_LINK:
        return kernel_linknode(target, source);

    case EVENT_UNLINK:
        return kernel_unlinknode(target, source);

    case EVENT_DATA:
        return ondata(interface, source, data, count);

    }

    return MESSAGE_UNIMPLEMENTED;

}

void console_notifydata(struct console_interface *interface, unsigned char data)
{

    struct event_consoledata consoledata;

    consoledata.data = data;

    kernel_notify(interface->inode, EVENT_CONSOLEDATA, sizeof (struct event_consoledata), &consoledata);

}

void console_registerinterface(struct console_interface *interface)
{

    resource_register(&interface->resource);

}

void console_unregisterinterface(struct console_interface *interface)
{

    resource_unregister(&interface->resource);

}

void console_initinterface(struct console_interface *interface, unsigned int id, unsigned int (*ondata)(unsigned int source, void *buffer, unsigned int count))
{

    resource_init(&interface->resource, RESOURCE_CONSOLEINTERFACE, interface);

    interface->id = id;
    interface->inode = kernel_addnode("console", &interface->resource, &service);
    interface->ondata = ondata;

}

void module_init(void)
{

    service_init(&service, 0, service_place);

}

