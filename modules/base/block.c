#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include "base.h"
#include "block.h"

static struct system_node root;

static unsigned int data_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_block_node *node = (struct base_block_node *)self->parent;

    return node->interface->read_data(node->device->bus, node->device->id, offset, count, buffer);

}

void base_block_register_interface(struct base_block_interface *interface)
{

    base_register_interface(&interface->base);

}

void base_block_register_protocol(struct base_block_protocol *protocol)
{

}

void base_block_register_node(struct base_block_node *node)
{

    system_add_child(&root, &node->base);
    system_add_child(&node->base, &node->data);

}

void base_block_unregister_interface(struct base_block_interface *interface)
{

    base_unregister_interface(&interface->base);

}

void base_block_unregister_protocol(struct base_block_protocol *protocol)
{

}

void base_block_unregister_node(struct base_block_node *node)
{

}

void base_block_init_interface(struct base_block_interface *interface, unsigned int (*read_data)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_data)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct base_block_interface));
    base_init_interface(&interface->base, BASE_INTERFACE_TYPE_BLOCK);

    interface->read_data = read_data;
    interface->write_data = write_data;

}

void base_block_init_protocol(struct base_block_protocol *protocol, char *name)
{

    memory_clear(protocol, sizeof (struct base_block_protocol));

    protocol->name = name;

}

void base_block_init_node(struct base_block_node *node, struct base_device *device, struct base_block_interface *interface)
{

    memory_clear(node, sizeof (struct base_block_node));
    system_init_multigroup(&node->base, device->bus->name);
    system_init_stream(&node->data, "data");

    node->device = device;
    node->interface = interface;
    node->data.read = data_read;

}

void base_block_setup()
{

    system_init_group(&root, "block");
    system_register_node(&root);

}

