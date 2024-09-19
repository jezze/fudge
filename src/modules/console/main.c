#include <fudge.h>
#include <kernel.h>
#include "console.h"

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
    kernel_announce(interface->ichannel, interface, 0, place);

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

