#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include "base.h"
#include "terminal.h"

static struct system_node root;

static unsigned int data_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_terminal_node *node = (struct base_terminal_node *)self->parent;
 
    return node->interface->read_data(node->device->bus, node->device->id, offset, count, buffer);

}

static unsigned int data_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_terminal_node *node = (struct base_terminal_node *)self->parent;

    return node->interface->write_data(node->device->bus, node->device->id, offset, count, buffer);

}

void base_terminal_register_interface(struct base_terminal_interface *interface)
{

    base_register_interface(&interface->base);

}

void base_terminal_register_node(struct base_terminal_node *node)
{

    system_add_child(&root, &node->base);
    system_add_child(&node->base, &node->data);

}

void base_terminal_unregister_interface(struct base_terminal_interface *interface)
{

    base_unregister_interface(&interface->base);

}

void base_terminal_unregister_node(struct base_terminal_node *node)
{

}

void base_terminal_init_interface(struct base_terminal_interface *interface, unsigned int (*read_data)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_data)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct base_terminal_interface));
    base_init_interface(&interface->base, BASE_INTERFACE_TYPE_TERMINAL);

    interface->read_data = read_data;
    interface->write_data = write_data;

}

void base_terminal_init_node(struct base_terminal_node *node, struct base_device *device, struct base_terminal_interface *interface)
{

    memory_clear(node, sizeof (struct base_terminal_node));
    system_init_group(&node->base, device->bus->name);
    system_init_stream(&node->data, "data");

    node->device = device;
    node->interface = interface;
    node->data.read = data_read;
    node->data.write = data_write;

}

void base_terminal_setup()
{

    system_init_group(&root, "terminal");
    system_register_node(&root);

}

