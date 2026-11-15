#include <fudge.h>
#include <kernel.h>
#include "power.h"

static struct unitsrv unitsrv;

static unsigned int operands_place(unsigned int target, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    struct resource *resource = pool_getnoderesource(target);

    if (resource)
    {

        struct power_interface *interface = resource->data;

        switch (event)
        {

        case MESSAGE_TYPE_POWEROFF:
            return (interface->poweroff) ? interface->poweroff() : MESSAGE_FAILED;

        case MESSAGE_TYPE_REBOOT:
            return (interface->reboot) ? interface->reboot() : MESSAGE_FAILED;

        }

    }

    return MESSAGE_FAILED;

}

void power_registerinterface(struct power_interface *interface)
{

    resource_register(&interface->resource);

}

void power_unregisterinterface(struct power_interface *interface)
{

    resource_unregister(&interface->resource);

}

void power_initinterface(struct power_interface *interface, unsigned int (*poweroff)(void), unsigned int (*reboot)(void))
{

    resource_init(&interface->resource, RESOURCE_POWERINTERFACE, interface, operands_place);

    interface->poweroff = poweroff;
    interface->reboot = reboot;

}

void module_init(void)
{

    unitsrv_init(&unitsrv, RESOURCE_POWERINTERFACE, "power");

}

void module_register(void)
{

    unitsrv_register(&unitsrv);

}

void module_unregister(void)
{

    unitsrv_unregister(&unitsrv);

}

