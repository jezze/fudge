#include <fudge/module.h>
#include <system/system.h>
#include "base.h"
#include "terminal.h"

struct terminal_device_node
{

    struct system_group base;
    struct base_terminal_interface *interface;
    struct base_device *device;
    struct system_stream data;

};

static struct system_group root;
static struct system_group dev;
static struct terminal_device_node dnodes[8];

static unsigned int data_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct terminal_device_node *node = (struct terminal_device_node *)self->parent;
 
    return node->interface->read_data(node->device, offset, count, buffer);

}

static unsigned int data_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct terminal_device_node *node = (struct terminal_device_node *)self->parent;

    return node->interface->write_data(node->device, offset, count, buffer);

}

static unsigned int find_device_node()
{

    unsigned int i;

    for (i = 1; i < 8; i++)
    {

        if (!dnodes[i].base.node.parent)
            return i;

    }

    return 0;

}

static void init_device_node(struct terminal_device_node *node, struct base_terminal_interface *interface, struct base_device *device)
{

    memory_clear(node, sizeof (struct terminal_device_node));
    system_init_group(&node->base, device->module.name);
    system_init_stream(&node->data, "data");

    node->interface = interface;
    node->device = device;
    node->data.node.read = data_read;
    node->data.node.write = data_write;

}

void base_terminal_register_interface(struct base_terminal_interface *interface, struct base_device *device)
{

    unsigned int index = find_device_node();

    if (!index)
        return;

    init_device_node(&dnodes[index], interface, device);
    system_group_add(&dnodes[index].base, &dnodes[index].data.node);
    system_group_add(&dev, &dnodes[index].base.node);

}

void base_terminal_init_interface(struct base_terminal_interface *interface, unsigned int (*read_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct base_terminal_interface));

    interface->read_data = read_data;
    interface->write_data = write_data;

}

void base_terminal_setup()
{

    system_init_group(&root, "terminal");
    system_init_group(&dev, "dev");
    system_group_add(&root, &dev.node);
    system_register_node(&root.node);

}

