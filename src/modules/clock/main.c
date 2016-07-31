#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
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

static unsigned int interfacectrl_read(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    struct clock_interface *interface = self->resource->data;
    struct ctrl_clocksettings *settings = buffer;

    interface->settings.seconds = interface->getseconds();
    interface->settings.minutes = interface->getminutes();
    interface->settings.hours = interface->gethours();
    interface->settings.weekday = interface->getweekday();
    interface->settings.day = interface->getday();
    interface->settings.month = interface->getmonth();
    interface->settings.year = interface->getyear();

    return memory_read(settings, count, &interface->settings, sizeof (struct ctrl_clocksettings), state->offset);

}

static unsigned int interfacetimestamp_read(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
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

    return memory_read(buffer, count, num, ascii_wvalue(num, 32, timestamp, 10, 0), state->offset);

}

static unsigned int interfacedate_read(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    struct clock_interface *interface = self->resource->data;
    char *num = "0000-00-00";

    ascii_wzerovalue(num, 10, interface->getyear(), 10, 4, 0);
    ascii_wzerovalue(num, 10, interface->getmonth(), 10, 2, 5);
    ascii_wzerovalue(num, 10, interface->getday(), 10, 2, 8);

    return memory_read(buffer, count, num, 10, state->offset);

}

static unsigned int interfacetime_read(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    struct clock_interface *interface = self->resource->data;
    char *num = "00:00:00";

    ascii_wzerovalue(num, 8, interface->gethours(), 10, 2, 0);
    ascii_wzerovalue(num, 8, interface->getminutes(), 10, 2, 3);
    ascii_wzerovalue(num, 8, interface->getseconds(), 10, 2, 6);

    return memory_read(buffer, count, num, 8, state->offset);

}

void clock_registerinterface(struct clock_interface *interface, unsigned int id)
{

    resource_register(&interface->resource);
    system_addchild(&interface->root, &interface->ctrl);
    system_addchild(&interface->root, &interface->timestamp);
    system_addchild(&interface->root, &interface->date);
    system_addchild(&interface->root, &interface->time);
    system_addchild(&root, &interface->root);

    interface->id = id;

}

void clock_unregisterinterface(struct clock_interface *interface)
{

    resource_unregister(&interface->resource);
    system_removechild(&interface->root, &interface->ctrl);
    system_removechild(&interface->root, &interface->timestamp);
    system_removechild(&interface->root, &interface->date);
    system_removechild(&interface->root, &interface->time);
    system_removechild(&root, &interface->root);

}

void clock_initinterface(struct clock_interface *interface, unsigned char (*getseconds)(void), unsigned char (*getminutes)(void), unsigned char (*gethours)(void), unsigned char (*getweekday)(void), unsigned char (*getday)(void), unsigned char (*getmonth)(void), unsigned short (*getyear)(void))
{

    resource_init(&interface->resource, RESOURCE_CLOCKINTERFACE, interface);
    system_initresourcenode(&interface->root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "if", &interface->resource);
    system_initresourcenode(&interface->ctrl, SYSTEM_NODETYPE_NORMAL, "ctrl", &interface->resource);
    system_initresourcenode(&interface->timestamp, SYSTEM_NODETYPE_NORMAL, "timestamp", &interface->resource);
    system_initresourcenode(&interface->date, SYSTEM_NODETYPE_NORMAL, "date", &interface->resource);
    system_initresourcenode(&interface->time, SYSTEM_NODETYPE_NORMAL, "time", &interface->resource);

    interface->getseconds = getseconds;
    interface->getminutes = getminutes;
    interface->gethours = gethours;
    interface->getweekday = getweekday;
    interface->getday = getday;
    interface->getmonth = getmonth;
    interface->getyear = getyear;
    interface->ctrl.read = interfacectrl_read;
    interface->timestamp.read = interfacetimestamp_read;
    interface->date.read = interfacedate_read;
    interface->time.read = interfacetime_read;

}

void module_init(void)
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "clock");

}

void module_register(void)
{

    system_registernode(&root);

}

void module_unregister(void)
{

    system_unregisternode(&root);

}

