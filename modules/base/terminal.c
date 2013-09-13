#include <fudge/module.h>
#include <system/system.h>
#include "base.h"
#include "terminal.h"

static struct system_group root;
static struct system_stream data;

static unsigned int data_open(struct system_node *self)
{

    return (unsigned int)self;

}

static unsigned int data_close(struct system_node *self)
{

    return (unsigned int)self;

}

static unsigned int data_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_terminal *interface = (struct base_terminal *)self->parent;
    struct base_device *device = (struct base_device *)self->parent->parent;
 
    return interface->read_data(device, offset, count, buffer);

}

static unsigned int data_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_terminal *interface = (struct base_terminal *)self->parent;
    struct base_device *device = (struct base_device *)self->parent->parent;

    return interface->write_data(device, offset, count, buffer);

}

void base_register_terminal(struct base_terminal *interface, struct base_device *device)
{

    system_group_add(&root, &device->module.base.node);
    system_group_add(&device->module.base, &interface->base.node);
    system_group_add(&interface->base, &data.node);

}

void base_init_terminal(struct base_terminal *interface, unsigned int (*read_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct base_terminal));
    system_init_group(&interface->base, "terminal");

    interface->read_data = read_data;
    interface->write_data = write_data;

}

void base_setup_terminal()
{

    system_init_group(&root, "terminal");
    system_init_stream(&data, "data", data_open, data_close, data_read, data_write);
    system_register_node(&root.node);

}

