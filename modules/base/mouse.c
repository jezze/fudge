#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include "base.h"
#include "mouse.h"

static struct system_group root;

static unsigned int data_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_mouse_node *node = (struct base_mouse_node *)self->parent;

    return node->interface->read_data(node->device->bus, node->device->id, offset, count, buffer);

}

void base_mouse_register_interface(struct base_mouse_interface *interface)
{

    base_register_interface(&interface->base);

}

void base_mouse_register_node(struct base_mouse_node *node)
{

    system_group_add(&root, &node->base.node);
    system_group_add(&node->base, &node->data.node);

}

void base_mouse_unregister_interface(struct base_mouse_interface *interface)
{

    base_unregister_interface(&interface->base);

}

void base_mouse_unregister_node(struct base_mouse_node *node)
{

}

void base_mouse_init_interface(struct base_mouse_interface *interface, unsigned int (*read_data)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct base_mouse_interface));
    base_init_interface(&interface->base, BASE_INTERFACE_TYPE_MOUSE);

    interface->read_data = read_data;

}

void base_mouse_init_node(struct base_mouse_node *node, struct base_device *device, struct base_mouse_interface *interface)
{

    memory_clear(node, sizeof (struct base_mouse_node));
    system_init_group(&node->base, device->bus->name);
    system_init_stream(&node->data, "data");

    node->device = device;
    node->interface = interface;
    node->data.node.read = data_read;

}

void base_mouse_setup()
{

    system_init_group(&root, "mouse");
    system_register_node(&root.node);

}

