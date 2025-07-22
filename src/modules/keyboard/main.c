#include <fudge.h>
#include <kernel.h>
#include "keyboard.h"

static struct service service;

static struct resource *service_foreach(struct resource *current)
{

    return resource_foreachtype(current, RESOURCE_KEYBOARDINTERFACE);

}

static unsigned int service_getinode(struct resource *current, unsigned int index)
{

    struct keyboard_interface *interface = current->data;

    return interface->inode;

}

static unsigned int service_place(unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    switch (event)
    {

    case EVENT_LINK:
        return kernel_linknode(target, source);

    case EVENT_UNLINK:
        return kernel_unlinknode(target, source);

    }

    return MESSAGE_UNIMPLEMENTED;

}

void keyboard_notifypress(struct keyboard_interface *interface, unsigned char scancode)
{

    struct event_keypress keypress;

    keypress.scancode = scancode;

    kernel_notify(interface->inode, EVENT_KEYPRESS, sizeof (struct event_keypress), &keypress);

}

void keyboard_notifyrelease(struct keyboard_interface *interface, unsigned char scancode)
{

    struct event_keyrelease keyrelease;

    keyrelease.scancode = scancode;

    kernel_notify(interface->inode, EVENT_KEYRELEASE, sizeof (struct event_keyrelease), &keyrelease);

}

void keyboard_registerinterface(struct keyboard_interface *interface)
{

    resource_register(&interface->resource);

}

void keyboard_unregisterinterface(struct keyboard_interface *interface)
{

    resource_unregister(&interface->resource);

}

void keyboard_initinterface(struct keyboard_interface *interface, unsigned int id)
{

    resource_init(&interface->resource, RESOURCE_KEYBOARDINTERFACE, interface);

    interface->id = id;
    interface->inode = kernel_addnode(&interface->resource, &service);

}

void module_init(void)
{

    service_init(&service, "keyboard", service_foreach, service_getinode, 0, 0, service_place);
    service_register(&service);

}

