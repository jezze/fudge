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

void timer_registerinterface(struct timer_interface *interface)
{

    base_registerinterface(&interface->base);

}

void timer_registerinterfacenode(struct timer_interfacenode *node)
{

    system_addchild(&root, &node->base);
    system_addchild(&node->base, &node->sleep);

}

void timer_unregisterinterface(struct timer_interface *interface)
{

    base_unregisterinterface(&interface->base);

}

void timer_unregisterinterfacenode(struct timer_interfacenode *node)
{

    system_removechild(&node->base, &node->sleep);
    system_removechild(&root, &node->base);

}

void timer_initinterface(struct timer_interface *interface, struct base_driver *driver, struct base_bus *bus, unsigned int id, void (*sleep)(unsigned int duration))
{

    memory_clear(interface, sizeof (struct timer_interface));
    base_initinterface(&interface->base, driver, bus, id);

    interface->sleep = sleep;

}

void timer_initinterfacenode(struct timer_interfacenode *node, struct timer_interface *interface)
{

    memory_clear(node, sizeof (struct timer_interfacenode));
    system_initnode(&node->base, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, interface->base.bus->name);
    system_initnode(&node->sleep, SYSTEM_NODETYPE_NORMAL, "sleep");

    node->interface = interface;
    node->sleep.read = interfacenode_sleepread;

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

