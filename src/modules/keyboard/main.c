#include <fudge.h>
#include <kernel.h>
#include "keyboard.h"

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
    kernel_announce(interface->ichannel, interface, interface->ichannel, place);

}

void keyboard_unregisterinterface(struct keyboard_interface *interface)
{

    resource_unregister(&interface->resource);
    kernel_unannounce(interface->ichannel);

}

void keyboard_initinterface(struct keyboard_interface *interface, unsigned int id, unsigned int ichannel)
{

    resource_init(&interface->resource, RESOURCE_KEYBOARDINTERFACE, interface);

    interface->id = id;
    interface->ichannel = ichannel;

}

