#include <fudge.h>
#include <kernel.h>
#include "battery.h"

static struct unitsrv unitsrv;

static unsigned int oninfo(struct battery_interface *interface, unsigned int source)
{

    struct message_batteryinfo batteryinfo;

    batteryinfo.present = interface->getpresent();
    batteryinfo.unit = interface->unit;
    batteryinfo.state = (batteryinfo.present) ? interface->getstate() : 0;
    batteryinfo.rate = (batteryinfo.present) ? interface->getrate() : MESSAGE_BATTERY_UNKNOWN;
    batteryinfo.remaining = (batteryinfo.present) ? interface->getremaining() : MESSAGE_BATTERY_UNKNOWN;
    batteryinfo.full = (batteryinfo.present) ? interface->getfull() : MESSAGE_BATTERY_UNKNOWN;
    batteryinfo.design = (batteryinfo.present) ? interface->getdesign() : MESSAGE_BATTERY_UNKNOWN;
    batteryinfo.voltage = (batteryinfo.present) ? interface->getvoltage() : MESSAGE_BATTERY_UNKNOWN;

    return kernel_placereply(source, interface->resource.inode, MESSAGE_TYPE_BATTERYINFO, sizeof (struct message_batteryinfo), &batteryinfo);

}

static unsigned int operands_place(unsigned int target, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    struct resource *resource = pool_getnoderesource(target);

    if (resource)
    {

        struct battery_interface *interface = resource->data;

        switch (event)
        {

        case MESSAGE_TYPE_LINK:
            return kernel_linknode(resource, source);

        case MESSAGE_TYPE_UNLINK:
            return kernel_unlinknode(resource, source);

        case MESSAGE_TYPE_INFO:
            return oninfo(interface, source);

        }

    }

    return MESSAGE_FAILED;

}

void battery_registerinterface(struct battery_interface *interface)
{

    resource_register(&interface->resource);

}

void battery_unregisterinterface(struct battery_interface *interface)
{

    resource_unregister(&interface->resource);

}

void battery_initinterface(struct battery_interface *interface, unsigned int unit, unsigned int (*getpresent)(void), unsigned int (*getstate)(void), unsigned int (*getrate)(void), unsigned int (*getremaining)(void), unsigned int (*getfull)(void), unsigned int (*getdesign)(void), unsigned int (*getvoltage)(void))
{

    resource_init(&interface->resource, RESOURCE_BATTERYINTERFACE, interface, operands_place);

    interface->unit = unit;
    interface->getpresent = getpresent;
    interface->getstate = getstate;
    interface->getrate = getrate;
    interface->getremaining = getremaining;
    interface->getfull = getfull;
    interface->getdesign = getdesign;
    interface->getvoltage = getvoltage;

}

void module_init(void)
{

    unitsrv_init(&unitsrv, RESOURCE_BATTERYINTERFACE, "battery");

}

void module_register(void)
{

    unitsrv_register(&unitsrv);

}

void module_unregister(void)
{

    unitsrv_unregister(&unitsrv);

}

