#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "clock.h"

static struct system_node root;

static unsigned int interfacectrl_read(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    struct clock_interface *interface = self->resource->data;

    interface->settings.seconds = interface->getseconds();
    interface->settings.minutes = interface->getminutes();
    interface->settings.hours = interface->gethours();
    interface->settings.weekday = interface->getweekday();
    interface->settings.day = interface->getday();
    interface->settings.month = interface->getmonth();
    interface->settings.year = interface->getyear();

    return memory_read(buffer, count, &interface->settings, sizeof (struct ctrl_clocksettings), offset);

}

void clock_registerinterface(struct clock_interface *interface, unsigned int id)
{

    resource_register(&interface->resource);
    system_addchild(&interface->root, &interface->ctrl);
    system_addchild(&root, &interface->root);

    interface->id = id;

}

void clock_unregisterinterface(struct clock_interface *interface)
{

    resource_unregister(&interface->resource);
    system_removechild(&interface->root, &interface->ctrl);
    system_removechild(&root, &interface->root);

}

void clock_initinterface(struct clock_interface *interface, unsigned char (*getseconds)(void), unsigned char (*getminutes)(void), unsigned char (*gethours)(void), unsigned char (*getweekday)(void), unsigned char (*getday)(void), unsigned char (*getmonth)(void), unsigned int (*getyear)(void))
{

    resource_init(&interface->resource, RESOURCE_CLOCKINTERFACE, interface);
    system_initresourcenode(&interface->root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "if", &interface->resource);
    system_initresourcenode(&interface->ctrl, SYSTEM_NODETYPE_NORMAL, "ctrl", &interface->resource);

    interface->getseconds = getseconds;
    interface->getminutes = getminutes;
    interface->gethours = gethours;
    interface->getweekday = getweekday;
    interface->getday = getday;
    interface->getmonth = getmonth;
    interface->getyear = getyear;
    interface->ctrl.read = interfacectrl_read;

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

