#include <fudge.h>
#include <kernel.h>
#include "mouse.h"

static struct node_operands operands;

static unsigned int operands_place(unsigned int target, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    struct resource *resource = pool_getnoderesource(target);

    if (resource)
    {

        struct mouse_interface *interface = resource->data;

        switch (event)
        {

        case EVENT_LINK:
            return kernel_linknode(&interface->service.links, source);

        case EVENT_UNLINK:
            return kernel_unlinknode(&interface->service.links, source);

        }

    }

    return MESSAGE_FAILED;

}

void mouse_notifymove(struct mouse_interface *interface, char relx, char rely)
{

    struct event_mousemove mousemove;

    mousemove.relx = relx;
    mousemove.rely = rely;

    kernel_notify(&interface->service.links, interface->service.inode, EVENT_MOUSEMOVE, sizeof (struct event_mousemove), &mousemove);

}

void mouse_notifyscroll(struct mouse_interface *interface, char relz)
{

    struct event_mousescroll mousescroll;

    mousescroll.relz = relz;

    kernel_notify(&interface->service.links, interface->service.inode, EVENT_MOUSESCROLL, sizeof (struct event_mousescroll), &mousescroll);

}

void mouse_notifypress(struct mouse_interface *interface, unsigned int button)
{

    struct event_mousepress mousepress;

    mousepress.button = button;

    kernel_notify(&interface->service.links, interface->service.inode, EVENT_MOUSEPRESS, sizeof (struct event_mousepress), &mousepress);

}

void mouse_notifyrelease(struct mouse_interface *interface, unsigned int button)
{

    struct event_mouserelease mouserelease;

    mouserelease.button = button;

    kernel_notify(&interface->service.links, interface->service.inode, EVENT_MOUSERELEASE, sizeof (struct event_mouserelease), &mouserelease);

}

void mouse_registerinterface(struct mouse_interface *interface)
{

    unsigned int inode = pool_picknode(&interface->resource, &operands);

    resource_register(&interface->resource);
    service_register(&interface->service, inode, "mouse");

}

void mouse_unregisterinterface(struct mouse_interface *interface)
{

    resource_unregister(&interface->resource);
    service_unregister(&interface->service);

}

void mouse_initinterface(struct mouse_interface *interface, unsigned int id)
{

    resource_init(&interface->resource, RESOURCE_MOUSEINTERFACE, interface);
    service_init(&interface->service);

    interface->id = id;

}

void module_init(void)
{

    node_operands_init(&operands, 0, operands_place);

}

