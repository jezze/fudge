#include <fudge.h>
#include <kernel.h>
#include "clock.h"

static struct node_operands operands;

static unsigned int oninfo(struct clock_interface *interface, unsigned int source)
{

    if (interface->getseconds && interface->getminutes && interface->gethours && interface->getweekday && interface->getday && interface->getmonth && interface->getyear)
    {

        struct event_clockinfo clockinfo;

        clockinfo.seconds = interface->getseconds();
        clockinfo.minutes = interface->getminutes();
        clockinfo.hours = interface->gethours();
        clockinfo.weekday = interface->getweekday();
        clockinfo.day = interface->getday();
        clockinfo.month = interface->getmonth();
        clockinfo.year = interface->getyear();

        return kernel_place(interface->service.inode, source, EVENT_CLOCKINFO, sizeof (struct event_clockinfo), &clockinfo);

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

        case EVENT_LINK:
            return kernel_linknode(&interface->service.links, source);

        case EVENT_UNLINK:
            return kernel_unlinknode(&interface->service.links, source);

        case EVENT_INFO:
            return oninfo(interface, source);

        }

    }

    return MESSAGE_FAILED;

}

void clock_registerinterface(struct clock_interface *interface)
{

    unsigned int inode = pool_picknode(&interface->resource, &operands);

    resource_register(&interface->resource);
    service_register(&interface->service, inode, "clock");

}

void clock_unregisterinterface(struct clock_interface *interface)
{

    resource_unregister(&interface->resource);
    service_unregister(&interface->service);

}

void clock_initinterface(struct clock_interface *interface, unsigned int id, unsigned int (*getseconds)(void), unsigned int (*getminutes)(void), unsigned int (*gethours)(void), unsigned int (*getweekday)(void), unsigned int (*getday)(void), unsigned int (*getmonth)(void), unsigned int (*getyear)(void))
{

    resource_init(&interface->resource, RESOURCE_CLOCKINTERFACE, interface);
    service_init(&interface->service);

    interface->id = id;
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

    node_operands_init(&operands, 0, operands_place);

}

