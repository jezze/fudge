#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include "base.h"
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

static unsigned int timestamp_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_clock_node *node = (struct base_clock_node *)self->parent;
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

static unsigned int date_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_clock_node *node = (struct base_clock_node *)self->parent;
    char *num = "0000-00-00";

    ascii_wzerovalue(num, 10, node->interface->getyear(), 10, 4, 0);
    ascii_wzerovalue(num, 10, node->interface->getmonth(), 10, 2, 5);
    ascii_wzerovalue(num, 10, node->interface->getday(), 10, 2, 8);

    return memory_read(buffer, count, num, 10, offset);

}

static unsigned int time_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_clock_node *node = (struct base_clock_node *)self->parent;
    char *num = "00:00:00";

    ascii_wzerovalue(num, 8, node->interface->gethours(), 10, 2, 0);
    ascii_wzerovalue(num, 8, node->interface->getminutes(), 10, 2, 3);
    ascii_wzerovalue(num, 8, node->interface->getseconds(), 10, 2, 6);

    return memory_read(buffer, count, num, 8, offset);

}

void base_clock_registerinterface(struct base_clock_interface *interface)
{

    base_registerinterface(&interface->base);

}

void base_clock_registernode(struct base_clock_node *node)
{

    system_addchild(&root, &node->base);
    system_addchild(&node->base, &node->timestamp);
    system_addchild(&node->base, &node->date);
    system_addchild(&node->base, &node->time);

}

void base_clock_unregisterinterface(struct base_clock_interface *interface)
{

    base_unregisterinterface(&interface->base);

}

void base_clock_unregisternode(struct base_clock_node *node)
{

}

void base_clock_initinterface(struct base_clock_interface *interface, struct base_bus *bus, unsigned int id, unsigned char (*getseconds)(), unsigned char (*getminutes)(), unsigned char (*gethours)(), unsigned char (*getweekday)(), unsigned char (*getday)(), unsigned char (*getmonth)(), unsigned short (*getyear)())
{

    memory_clear(interface, sizeof (struct base_clock_interface));
    base_initinterface(&interface->base, bus, id);

    interface->getseconds = getseconds;
    interface->getminutes = getminutes;
    interface->gethours = gethours;
    interface->getweekday = getweekday;
    interface->getday = getday;
    interface->getmonth = getmonth;
    interface->getyear = getyear;

}

void base_clock_initnode(struct base_clock_node *node, struct base_clock_interface *interface)
{

    memory_clear(node, sizeof (struct base_clock_node));
    system_initnode(&node->base, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, interface->base.bus->name);
    system_initnode(&node->timestamp, SYSTEM_NODETYPE_NORMAL, "timestamp");
    system_initnode(&node->date, SYSTEM_NODETYPE_NORMAL, "date");
    system_initnode(&node->time, SYSTEM_NODETYPE_NORMAL, "time");

    node->interface = interface;
    node->timestamp.read = timestamp_read;
    node->date.read = date_read;
    node->time.read = time_read;

}

void base_clock_setup()
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "clock");
    system_registernode(&root);

}

