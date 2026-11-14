#include <fudge.h>
#include <kernel.h>
#include "clock.h"

static struct unitsrv unitsrv;

static unsigned int oninfo(struct clock_interface *interface, unsigned int source)
{

    if (interface->getseconds && interface->getminutes && interface->gethours && interface->getweekday && interface->getday && interface->getmonth && interface->getyear)
    {

        struct message_clockinfo clockinfo;

        clockinfo.seconds = interface->getseconds();
        clockinfo.minutes = interface->getminutes();
        clockinfo.hours = interface->gethours();
        clockinfo.weekday = interface->getweekday();
        clockinfo.day = interface->getday();
        clockinfo.month = interface->getmonth();
        clockinfo.year = interface->getyear();

        return kernel_place(interface->resource.inode, source, MESSAGE_TYPE_CLOCKINFO, sizeof (struct message_clockinfo), &clockinfo);

    }

    return MESSAGE_FAILED;

}

static unsigned int operands_place(unsigned int target, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    struct resource *resource = pool_getnoderesource(target);

    if (resource)
    {

        struct clock_interface *interface = resource->data;

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

void clock_registerinterface(struct clock_interface *interface)
{

    resource_register(&interface->resource);

}

void clock_unregisterinterface(struct clock_interface *interface)
{

    resource_unregister(&interface->resource);

}

void clock_initinterface(struct clock_interface *interface, unsigned int (*getseconds)(void), unsigned int (*getminutes)(void), unsigned int (*gethours)(void), unsigned int (*getweekday)(void), unsigned int (*getday)(void), unsigned int (*getmonth)(void), unsigned int (*getyear)(void))
{

    resource_init(&interface->resource, RESOURCE_CLOCKINTERFACE, interface, operands_place);

    interface->getseconds = getseconds;
    interface->getminutes = getminutes;
    interface->gethours = gethours;
    interface->getweekday = getweekday;
    interface->getday = getday;
    interface->getmonth = getmonth;
    interface->getyear = getyear;

}

void module_init(void)
{

    unitsrv_init(&unitsrv, RESOURCE_CLOCKINTERFACE, "clock");

}

void module_register(void)
{

    unitsrv_register(&unitsrv);

}

void module_unregister(void)
{

    unitsrv_unregister(&unitsrv);

}

