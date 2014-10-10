#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include "base.h"
#include "timer.h"

static struct system_node root;

void base_timer_registerinterface(struct base_timer_interface *interface)
{

    base_registerinterface(&interface->base);

}

void base_timer_registernode(struct base_timer_node *node)
{

    system_addchild(&root, &node->base);

}

void base_timer_unregisterinterface(struct base_timer_interface *interface)
{

    base_unregisterinterface(&interface->base);

}

void base_timer_unregisternode(struct base_timer_node *node)
{

}

void base_timer_initinterface(struct base_timer_interface *interface, struct base_bus *bus, unsigned int id, void (*addduration)(unsigned int duration))
{

    memory_clear(interface, sizeof (struct base_timer_interface));
    base_initinterface(&interface->base, bus, id);

    interface->addduration = addduration;

}

void base_timer_initnode(struct base_timer_node *node, struct base_timer_interface *interface)
{

    memory_clear(node, sizeof (struct base_timer_node));
    system_initnode(&node->base, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, interface->base.bus->name);

    node->interface = interface;

}

void base_timer_setup()
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "timer");
    system_registernode(&root);

}

