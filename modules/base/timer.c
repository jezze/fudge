#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include "base.h"
#include "timer.h"

static struct system_node root;

static unsigned int sleep_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_timer_interfacenode *node = (struct base_timer_interfacenode *)self->parent;

    /* 10s */
    node->interface->sleep(1000);

    return 0;

}

void base_timer_registerinterface(struct base_timer_interface *interface)
{

    base_registerinterface(&interface->base);

}

void base_timer_registerinterfacenode(struct base_timer_interfacenode *node)
{

    system_addchild(&root, &node->base);
    system_addchild(&node->base, &node->sleep);

}

void base_timer_unregisterinterface(struct base_timer_interface *interface)
{

    base_unregisterinterface(&interface->base);

}

void base_timer_unregisterinterfacenode(struct base_timer_interfacenode *node)
{

    system_removechild(&node->base, &node->sleep);
    system_removechild(&root, &node->base);

}

void base_timer_initinterface(struct base_timer_interface *interface, struct base_bus *bus, unsigned int id, void (*sleep)(unsigned int duration))
{

    memory_clear(interface, sizeof (struct base_timer_interface));
    base_initinterface(&interface->base, bus, id);

    interface->sleep = sleep;

}

void base_timer_initinterfacenode(struct base_timer_interfacenode *node, struct base_timer_interface *interface)
{

    memory_clear(node, sizeof (struct base_timer_interfacenode));
    system_initnode(&node->base, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, interface->base.bus->name);
    system_initnode(&node->sleep, SYSTEM_NODETYPE_NORMAL, "sleep");

    node->interface = interface;
    node->sleep.read = sleep_read;

}

void base_timer_setup()
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "timer");
    system_registernode(&root);

}

