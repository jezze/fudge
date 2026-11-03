#include <fudge.h>
#include <kernel.h>
#include "keyboard.h"

static struct resource_operands operands;
static struct dirsrv dirsrv;
static struct service service;

static unsigned int operands_place(unsigned int target, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    struct resource *resource = pool_getnoderesource(target);

    if (resource)
    {

        struct keyboard_interface *interface = resource->data;

        switch (event)
        {

        case EVENT_LINK:
            return kernel_linknode(&interface->resource, source);

        case EVENT_UNLINK:
            return kernel_unlinknode(&interface->resource, source);

        }

    }

    return MESSAGE_FAILED;

}

void keyboard_notifypress(struct keyboard_interface *interface, unsigned char scancode)
{

    struct event_keypress keypress;

    keypress.scancode = scancode;

    kernel_notify(&interface->resource, EVENT_KEYPRESS, sizeof (struct event_keypress), &keypress);

}

void keyboard_notifyrelease(struct keyboard_interface *interface, unsigned char scancode)
{

    struct event_keyrelease keyrelease;

    keyrelease.scancode = scancode;

    kernel_notify(&interface->resource, EVENT_KEYRELEASE, sizeof (struct event_keyrelease), &keyrelease);

}

void keyboard_registerinterface(struct keyboard_interface *interface)
{

    resource_register(&interface->resource);

}

void keyboard_unregisterinterface(struct keyboard_interface *interface)
{

    resource_unregister(&interface->resource);

}

void keyboard_initinterface(struct keyboard_interface *interface)
{

    resource_init(&interface->resource, RESOURCE_KEYBOARDINTERFACE, interface, &operands);

}

void module_init(void)
{

    resource_operands_init(&operands, 0, operands_place, 0);

    dirsrv_init(&dirsrv, RESOURCE_KEYBOARDINTERFACE);
    service_init(&service);

}

void module_register(void)
{

    service_register(&service, pool_attachnode(&dirsrv.resource), "keyboard");

}

void module_unregister(void)
{

    service_unregister(&service);
    pool_detachnode(&dirsrv.resource);

}

