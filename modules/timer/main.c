#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include <base/base.h>
#include "timer.h"

static struct system_node root;

static unsigned int interfacenode_sleepread(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct timer_interfacenode *node = (struct timer_interfacenode *)self->parent;

    /* 10s */
    node->interface->sleep(1000);

    return 0;

}

void timer_registerinterface(struct timer_interface *interface, struct base_bus *bus, unsigned int id)
{

    base_registerinterface(&interface->base, bus, id);
    system_addchild(&root, &interface->node.base);
    system_addchild(&interface->node.base, &interface->node.sleep);

}

void timer_unregisterinterface(struct timer_interface *interface)
{

    base_unregisterinterface(&interface->base);
    system_removechild(&interface->node.base, &interface->node.sleep);
    system_removechild(&root, &interface->node.base);

}

void timer_initinterface(struct timer_interface *interface, struct base_driver *driver, void (*sleep)(unsigned int duration))
{

    base_initinterface(&interface->base, driver);
    system_initnode(&interface->node.base, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, driver->name);
    system_initnode(&interface->node.sleep, SYSTEM_NODETYPE_NORMAL, "sleep");

    interface->sleep = sleep;
    interface->node.interface = interface;
    interface->node.sleep.read = interfacenode_sleepread;

}

void init()
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "timer");
    system_registernode(&root);

}

void destroy()
{

    system_unregisternode(&root);

}

