#include <fudge.h>
#include <kernel.h>
#include "touch.h"

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

/* one finger of a touch surface in its own units (0 to xmax and ymax), the contact count and the surface's button with each report, time in the driver's ticks */
void touch_notifycontact(struct touch_interface *interface, unsigned int id, unsigned int tip, unsigned int x, unsigned int y, unsigned int xmax, unsigned int ymax, unsigned int count, unsigned int button, unsigned int time)
{

    struct message_touchcontact touchcontact;

    touchcontact.x = x;
    touchcontact.y = y;
    touchcontact.xmax = xmax;
    touchcontact.ymax = ymax;
    touchcontact.id = id;
    touchcontact.tip = tip;
    touchcontact.count = count;
    touchcontact.button = button;
    touchcontact.time = time;

    kernel_notify(&interface->resource, MESSAGE_TYPE_TOUCHCONTACT, sizeof (struct message_touchcontact), &touchcontact);

}

void touch_registerinterface(struct touch_interface *interface)
{

    resource_register(&interface->resource);

}

void touch_unregisterinterface(struct touch_interface *interface)
{

    resource_unregister(&interface->resource);

}

void touch_initinterface(struct touch_interface *interface)
{

    resource_init(&interface->resource, RESOURCE_TOUCHINTERFACE, interface, &operands);

}

void module_init(void)
{

    resource_operands_init(&operands, operands_place);

    unitsrv_init(&unitsrv, RESOURCE_TOUCHINTERFACE, "touch");

}

void module_register(void)
{

    unitsrv_register(&unitsrv);

}

void module_unregister(void)
{

    unitsrv_unregister(&unitsrv);

}

