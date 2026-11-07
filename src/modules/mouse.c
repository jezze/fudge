#include <fudge.h>
#include <kernel.h>
#include "mouse.h"

static struct resource_operands operands;
static struct unitsrv unitsrv;

static unsigned int operands_place(unsigned int target, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    struct resource *resource = pool_getnoderesource(target);

    if (resource)
    {

        struct mouse_interface *interface = resource->data;

        switch (event)
        {

        case MESSAGE_TYPE_LINK:
            return kernel_linknode(&interface->resource, source);

        case MESSAGE_TYPE_UNLINK:
            return kernel_unlinknode(&interface->resource, source);

        }

    }

    return MESSAGE_FAILED;

}

void mouse_notifymove(struct mouse_interface *interface, char relx, char rely)
{

    struct message_mousemove mousemove;

    mousemove.relx = relx;
    mousemove.rely = rely;
    mousemove.buttons = interface->buttons;

    kernel_notify(&interface->resource, MESSAGE_TYPE_MOUSEMOVE, sizeof (struct message_mousemove), &mousemove);

}

/* an absolute position, 0 to 65535 across each axis of whatever the device covers */
void mouse_notifyposition(struct mouse_interface *interface, unsigned short x, unsigned short y)
{

    struct message_mouseposition mouseposition;

    mouseposition.x = x;
    mouseposition.y = y;
    mouseposition.buttons = interface->buttons;

    kernel_notify(&interface->resource, MESSAGE_TYPE_MOUSEPOSITION, sizeof (struct message_mouseposition), &mouseposition);

}

void mouse_notifyscroll(struct mouse_interface *interface, char relz)
{

    struct message_mousescroll mousescroll;

    mousescroll.relz = relz;

    kernel_notify(&interface->resource, MESSAGE_TYPE_MOUSESCROLL, sizeof (struct message_mousescroll), &mousescroll);

}

void mouse_notifypress(struct mouse_interface *interface, unsigned int button)
{

    struct message_mousepress mousepress;

    mousepress.button = button;
    interface->buttons |= 1 << (button - 1);

    kernel_notify(&interface->resource, MESSAGE_TYPE_MOUSEPRESS, sizeof (struct message_mousepress), &mousepress);

}

void mouse_notifyrelease(struct mouse_interface *interface, unsigned int button)
{

    struct message_mouserelease mouserelease;

    mouserelease.button = button;
    interface->buttons &= ~(1 << (button - 1));

    kernel_notify(&interface->resource, MESSAGE_TYPE_MOUSERELEASE, sizeof (struct message_mouserelease), &mouserelease);

}

void mouse_registerinterface(struct mouse_interface *interface)
{

    resource_register(&interface->resource);

}

void mouse_unregisterinterface(struct mouse_interface *interface)
{

    resource_unregister(&interface->resource);

}

void mouse_initinterface(struct mouse_interface *interface)
{

    resource_init(&interface->resource, RESOURCE_MOUSEINTERFACE, interface, &operands);

}

void module_init(void)
{

    resource_operands_init(&operands, 0, operands_place, 0);

    unitsrv_init(&unitsrv, RESOURCE_MOUSEINTERFACE, "mouse");

}

void module_register(void)
{

    unitsrv_register(&unitsrv);

}

void module_unregister(void)
{

    unitsrv_unregister(&unitsrv);

}

