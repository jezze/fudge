#include <fudge/module.h>
#include <system/system.h>
#include "base.h"
#include "keyboard.h"

static struct system_group root;
static struct system_stream data;
static struct system_stream keymap;

static unsigned int data_open(struct system_node *self)
{

    return 1;

}

static unsigned int data_close(struct system_node *self)
{

    return 1;

}

static unsigned int data_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_keyboard *interface = (struct base_keyboard *)self->parent;
    struct base_device *device = (struct base_device *)self->parent->parent;
 
    return interface->read_data(device, offset, count, buffer);

}

static unsigned int data_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_keyboard *interface = (struct base_keyboard *)self->parent;
    struct base_device *device = (struct base_device *)self->parent->parent;
 
    return interface->write_data(device, offset, count, buffer);

}

static unsigned int keymap_open(struct system_node *self)
{

    return 1;

}

static unsigned int keymap_close(struct system_node *self)
{

    return 1;

}

static unsigned int keymap_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_keyboard *interface = (struct base_keyboard *)self->parent;

    return memory_read(buffer, count, interface->keymap, 256, offset);

}

static unsigned int keymap_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_keyboard *interface = (struct base_keyboard *)self->parent;

    return memory_write(interface->keymap, 256, buffer, count, offset);

}

void base_register_keyboard(struct base_keyboard *interface, struct base_device *device)
{

    system_group_add(&root, &device->module.base.node);
    system_group_add(&device->module.base, &interface->base.node);
    system_group_add(&interface->base, &data.node);
    system_group_add(&interface->base, &keymap.node);

}

void base_init_keyboard(struct base_keyboard *interface, unsigned int (*read_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct base_keyboard));
    system_init_group(&interface->base, "keyboard");

    interface->read_data = read_data;
    interface->write_data = write_data;

}

void base_setup_keyboard()
{

    system_init_group(&root, "keyboard");
    system_init_stream(&data, "data", data_open, data_close, data_read, data_write);
    system_init_stream(&keymap, "keymap", keymap_open, keymap_close, keymap_read, keymap_write);
    system_register_node(&root.node);

}

