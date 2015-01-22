#include <fudge.h>
#include <kernel.h>
#include <system/system.h>
#include <base/base.h>
#include "clock.h"

static struct system_node root;
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

static unsigned int interfacenode_timestampread(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct clock_interfacenode *node = (struct clock_interfacenode *)self->parent;
    unsigned int year = node->interface->getyear() - 1970;
    unsigned int month = node->interface->getmonth();
    unsigned int day = node->interface->getday();
    unsigned int hour = node->interface->gethours();
    unsigned int minute = node->interface->getminutes();
    unsigned int second = node->interface->getseconds();
    unsigned int dyear = ((((365 * year) + (year / 4)) - (year / 100)) + (year / 400));
    unsigned int dmonth = isleapyear(year) ? dotm366[month - 1] : dotm365[month - 1];
    unsigned int timestamp = ((dyear + dmonth + day) * 86400) + ((hour * 3600) + (minute * 60) + second);
    char num[32];

    return memory_read(buffer, count, num, ascii_wvalue(num, 32, timestamp, 10, 0), offset);

}

static unsigned int interfacenode_dateread(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct clock_interfacenode *node = (struct clock_interfacenode *)self->parent;
    char *num = "0000-00-00";

    ascii_wzerovalue(num, 10, node->interface->getyear(), 10, 4, 0);
    ascii_wzerovalue(num, 10, node->interface->getmonth(), 10, 2, 5);
    ascii_wzerovalue(num, 10, node->interface->getday(), 10, 2, 8);

    return memory_read(buffer, count, num, 10, offset);

}

static unsigned int interfacenode_timeread(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct clock_interfacenode *node = (struct clock_interfacenode *)self->parent;
    char *num = "00:00:00";

    ascii_wzerovalue(num, 8, node->interface->gethours(), 10, 2, 0);
    ascii_wzerovalue(num, 8, node->interface->getminutes(), 10, 2, 3);
    ascii_wzerovalue(num, 8, node->interface->getseconds(), 10, 2, 6);

    return memory_read(buffer, count, num, 8, offset);

}

void clock_registerinterface(struct clock_interface *interface, struct base_bus *bus, unsigned int id)
{

    base_registerinterface(&interface->base, bus, id);
    system_addchild(&root, &interface->node.base);
    system_addchild(&interface->node.base, &interface->node.timestamp);
    system_addchild(&interface->node.base, &interface->node.date);
    system_addchild(&interface->node.base, &interface->node.time);

}

void clock_unregisterinterface(struct clock_interface *interface)
{

    base_unregisterinterface(&interface->base);
    system_removechild(&interface->node.base, &interface->node.timestamp);
    system_removechild(&interface->node.base, &interface->node.date);
    system_removechild(&interface->node.base, &interface->node.time);
    system_removechild(&root, &interface->node.base);

}

void clock_initinterface(struct clock_interface *interface, struct base_driver *driver, unsigned char (*getseconds)(), unsigned char (*getminutes)(), unsigned char (*gethours)(), unsigned char (*getweekday)(), unsigned char (*getday)(), unsigned char (*getmonth)(), unsigned short (*getyear)())
{

    base_initinterface(&interface->base, driver);
    system_initnode(&interface->node.base, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, driver->name);
    system_initnode(&interface->node.timestamp, SYSTEM_NODETYPE_NORMAL, "timestamp");
    system_initnode(&interface->node.date, SYSTEM_NODETYPE_NORMAL, "date");
    system_initnode(&interface->node.time, SYSTEM_NODETYPE_NORMAL, "time");

    interface->getseconds = getseconds;
    interface->getminutes = getminutes;
    interface->gethours = gethours;
    interface->getweekday = getweekday;
    interface->getday = getday;
    interface->getmonth = getmonth;
    interface->getyear = getyear;
    interface->node.interface = interface;
    interface->node.timestamp.read = interfacenode_timestampread;
    interface->node.date.read = interfacenode_dateread;
    interface->node.time.read = interfacenode_timeread;

}

void init()
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "clock");
    system_registernode(&root);

}

void destroy()
{

    system_unregisternode(&root);

}

