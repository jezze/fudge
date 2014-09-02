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

void base_timer_initinterface(struct base_timer_interface *interface, void (*addduration)(struct base_bus *bus, unsigned int id, unsigned int duration))
{

    memory_clear(interface, sizeof (struct base_timer_interface));
    base_initinterface(&interface->base, BASE_INTERFACE_TYPE_TIMER);

    interface->addduration = addduration;

}

void base_timer_initnode(struct base_timer_node *node, struct base_device *device, struct base_timer_interface *interface)
{

    memory_clear(node, sizeof (struct base_timer_node));
    system_initmultigroup(&node->base, device->bus->name);

    node->device = device;
    node->interface = interface;

}

void base_timer_setup()
{

    system_initgroup(&root, "timer");
    system_registernode(&root);

}

