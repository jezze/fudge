#include <fudge.h>
#include <kernel.h>
#include "mouse.h"

static struct service service;

static unsigned int service_match(unsigned int count, char *name)
{

    if (count == 2 && buffer_match(name, ":", 1))
    {

        struct resource *current = 0;
        unsigned int index = cstring_toint(name[1]);
        unsigned int i;

        for (i = 0; (current = resource_foreachtype(current, RESOURCE_MOUSEINTERFACE)); i++)
        {

            struct mouse_interface *interface = current->data;

            if (i == index)
                return interface->ichannel;

        }

    }

    return 0;

}

static unsigned int place(void *interface, unsigned int ichannel, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    switch (event)
    {

    case EVENT_LINK:
        return kernel_link(ichannel, source);

    case EVENT_UNLINK:
        return kernel_unlink(ichannel, source);

    }

    return MESSAGE_UNIMPLEMENTED;

}

void mouse_notifymove(struct mouse_interface *interface, char relx, char rely)
{

    struct event_mousemove mousemove;

    mousemove.relx = relx;
    mousemove.rely = rely;

    kernel_notify(interface->ichannel, EVENT_MOUSEMOVE, sizeof (struct event_mousemove), &mousemove);

}

void mouse_notifyscroll(struct mouse_interface *interface, char relz)
{

    struct event_mousescroll mousescroll;

    mousescroll.relz = relz;

    kernel_notify(interface->ichannel, EVENT_MOUSESCROLL, sizeof (struct event_mousescroll), &mousescroll);

}

void mouse_notifypress(struct mouse_interface *interface, unsigned int button)
{

    struct event_mousepress mousepress;

    mousepress.button = button;

    kernel_notify(interface->ichannel, EVENT_MOUSEPRESS, sizeof (struct event_mousepress), &mousepress);

}

void mouse_notifyrelease(struct mouse_interface *interface, unsigned int button)
{

    struct event_mouserelease mouserelease;

    mouserelease.button = button;

    kernel_notify(interface->ichannel, EVENT_MOUSERELEASE, sizeof (struct event_mouserelease), &mouserelease);

}

void mouse_registerinterface(struct mouse_interface *interface)
{

    resource_register(&interface->resource);

    interface->ichannel = kernel_announce(0, interface, place);

}

void mouse_unregisterinterface(struct mouse_interface *interface)
{

    resource_unregister(&interface->resource);
    kernel_unannounce(interface->ichannel);

}

void mouse_initinterface(struct mouse_interface *interface, unsigned int id)
{

    resource_init(&interface->resource, RESOURCE_MOUSEINTERFACE, interface);

    interface->id = id;

}

void module_init(void)
{

    service_init(&service, "mouse", service_match);
    resource_register(&service.resource);

}

