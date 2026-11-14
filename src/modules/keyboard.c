#include <fudge.h>
#include <kernel.h>
#include "keyboard.h"

static struct resource_operands operands;
static struct unitsrv unitsrv;

static unsigned int operands_place(unsigned int target, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    struct resource *resource = pool_getnoderesource(target);

    if (resource)
    {

        switch (event)
        {

        case MESSAGE_TYPE_LINK:
            return kernel_linknode(resource, source);

        case MESSAGE_TYPE_UNLINK:
            return kernel_unlinknode(resource, source);

        }

    }

    return MESSAGE_FAILED;

}

void keyboard_notifypress(struct keyboard_interface *interface, unsigned char scancode)
{

    struct message_keypress keypress;

    keypress.scancode = scancode;

    kernel_notify(&interface->resource, MESSAGE_TYPE_KEYPRESS, sizeof (struct message_keypress), &keypress);

}

void keyboard_notifyrelease(struct keyboard_interface *interface, unsigned char scancode)
{

    struct message_keyrelease keyrelease;

    keyrelease.scancode = scancode;

    kernel_notify(&interface->resource, MESSAGE_TYPE_KEYRELEASE, sizeof (struct message_keyrelease), &keyrelease);

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

    resource_operands_init(&operands, operands_place, 0);

    unitsrv_init(&unitsrv, RESOURCE_KEYBOARDINTERFACE, "keyboard");

}

void module_register(void)
{

    unitsrv_register(&unitsrv);

}

void module_unregister(void)
{

    unitsrv_unregister(&unitsrv);

}

