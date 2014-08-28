#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include "base.h"
#include "timer.h"

static struct system_node root;

void base_timer_register_interface(struct base_timer_interface *interface)
{

    base_register_interface(&interface->base);

}

void base_timer_register_node(struct base_timer_node *node)
{

    system_add_child(&root, &node->base);

}

void base_timer_unregister_interface(struct base_timer_interface *interface)
{

    base_unregister_interface(&interface->base);

}

void base_timer_unregister_node(struct base_timer_node *node)
{

}

void base_timer_init_interface(struct base_timer_interface *interface, void (*add_duration)(struct base_bus *bus, unsigned int id, unsigned int duration))
{

    memory_clear(interface, sizeof (struct base_timer_interface));
    base_init_interface(&interface->base, BASE_INTERFACE_TYPE_TIMER);

    interface->add_duration = add_duration;

}

void base_timer_init_node(struct base_timer_node *node, struct base_device *device, struct base_timer_interface *interface)
{

    memory_clear(node, sizeof (struct base_timer_node));
    system_init_group(&node->base, device->bus->name);

    node->device = device;
    node->interface = interface;

}

void base_timer_setup()
{

    system_init_group(&root, "timer");
    system_register_node(&root);

}

