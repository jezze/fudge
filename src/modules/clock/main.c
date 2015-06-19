#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "clock.h"

static unsigned int dotm365[13] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
static unsigned int dotm366[13] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 365};

static unsigned int isleapyear(unsigned short year)
{

    if ((year % 4) != 0)
        return 0;

    if ((year % 100) == 0)
        return ((year % 400) == 0);

    return 1;

}

static unsigned int interfacenode_timestampread(struct system_node *self, unsigned int offset, unsigned int size, unsigned int count, void *buffer)
{

    struct clock_interface *interface = self->resource->data;
    unsigned int year = interface->getyear() - 1970;
    unsigned int month = interface->getmonth();
    unsigned int day = interface->getday();
    unsigned int hour = interface->gethours();
    unsigned int minute = interface->getminutes();
    unsigned int second = interface->getseconds();
    unsigned int dyear = ((((365 * year) + (year / 4)) - (year / 100)) + (year / 400));
    unsigned int dmonth = isleapyear(year) ? dotm366[month - 1] : dotm365[month - 1];
    unsigned int timestamp = ((dyear + dmonth + day) * 86400) + ((hour * 3600) + (minute * 60) + second);
    char num[32];

    return memory_read(buffer, count, num, ascii_wvalue(num, 32, timestamp, 10, 0), size, offset);

}

static unsigned int interfacenode_dateread(struct system_node *self, unsigned int offset, unsigned int size, unsigned int count, void *buffer)
{

    struct clock_interface *interface = self->resource->data;
    char *num = "0000-00-00";

    ascii_wzerovalue(num, 10, interface->getyear(), 10, 4, 0);
    ascii_wzerovalue(num, 10, interface->getmonth(), 10, 2, 5);
    ascii_wzerovalue(num, 10, interface->getday(), 10, 2, 8);

    return memory_read(buffer, count, num, 10, size, offset);

}

static unsigned int interfacenode_timeread(struct system_node *self, unsigned int offset, unsigned int size, unsigned int count, void *buffer)
{

    struct clock_interface *interface = self->resource->data;
    char *num = "00:00:00";

    ascii_wzerovalue(num, 8, interface->gethours(), 10, 2, 0);
    ascii_wzerovalue(num, 8, interface->getminutes(), 10, 2, 3);
    ascii_wzerovalue(num, 8, interface->getseconds(), 10, 2, 6);

    return memory_read(buffer, count, num, 8, size, offset);

}

void clock_registerinterface(struct clock_interface *interface, unsigned int id)
{

    system_addchild(&interface->root, &interface->timestamp);
    system_addchild(&interface->root, &interface->date);
    system_addchild(&interface->root, &interface->time);
    system_registernode(&interface->root);

    interface->id = id;

}

void clock_unregisterinterface(struct clock_interface *interface)
{

    system_removechild(&interface->root, &interface->timestamp);
    system_removechild(&interface->root, &interface->date);
    system_removechild(&interface->root, &interface->time);
    system_unregisternode(&interface->root);

}

void clock_initinterface(struct clock_interface *interface, unsigned char (*getseconds)(), unsigned char (*getminutes)(), unsigned char (*gethours)(), unsigned char (*getweekday)(), unsigned char (*getday)(), unsigned char (*getmonth)(), unsigned short (*getyear)())
{

    resource_init(&interface->resource, RESOURCE_CLOCKINTERFACE, interface);
    system_initnode(&interface->root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "clock");
    system_initnode(&interface->timestamp, SYSTEM_NODETYPE_NORMAL, "timestamp");
    system_initnode(&interface->date, SYSTEM_NODETYPE_NORMAL, "date");
    system_initnode(&interface->time, SYSTEM_NODETYPE_NORMAL, "time");

    interface->getseconds = getseconds;
    interface->getminutes = getminutes;
    interface->gethours = gethours;
    interface->getweekday = getweekday;
    interface->getday = getday;
    interface->getmonth = getmonth;
    interface->getyear = getyear;
    interface->timestamp.resource = &interface->resource;
    interface->timestamp.read = interfacenode_timestampread;
    interface->date.resource = &interface->resource;
    interface->date.read = interfacenode_dateread;
    interface->time.resource = &interface->resource;
    interface->time.read = interfacenode_timeread;

}

