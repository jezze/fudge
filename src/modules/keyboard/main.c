#include <fudge.h>
#include <kernel.h>
#include "keyboard.h"

static struct service service;

static unsigned int service_match(unsigned int count, char *name)
{

    if (count == 2 && buffer_match(name, ":", 1))
    {

        struct resource *current = 0;
        unsigned int index = cstring_toint(name[1]);
        unsigned int i;

        for (i = 0; (current = resource_foreachtype(current, RESOURCE_KEYBOARDINTERFACE)); i++)
        {

            struct keyboard_interface *interface = current->data;

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

void keyboard_notifypress(struct keyboard_interface *interface, unsigned char scancode)
{

    struct event_keypress keypress;

    keypress.scancode = scancode;

    kernel_notify(interface->ichannel, EVENT_KEYPRESS, sizeof (struct event_keypress), &keypress);

}

void keyboard_notifyrelease(struct keyboard_interface *interface, unsigned char scancode)
{

    struct event_keyrelease keyrelease;

    keyrelease.scancode = scancode;

    kernel_notify(interface->ichannel, EVENT_KEYRELEASE, sizeof (struct event_keyrelease), &keyrelease);

}

void keyboard_registerinterface(struct keyboard_interface *interface)
{

    resource_register(&interface->resource);

    interface->ichannel = kernel_announce(0, interface, place);

}

void keyboard_unregisterinterface(struct keyboard_interface *interface)
{

    resource_unregister(&interface->resource);

    kernel_unannounce(interface->ichannel);

}

void keyboard_initinterface(struct keyboard_interface *interface, unsigned int id)
{

    resource_init(&interface->resource, RESOURCE_KEYBOARDINTERFACE, interface);

    interface->id = id;

}

void module_init(void)
{

    service_init(&service, "keyboard", service_match);
    resource_register(&service.resource);

}

