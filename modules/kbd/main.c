#include <fudge/module.h>
#include <system/system.h>
#include <base/base.h>
#include "kbd.h"

static struct system_group root;
static struct system_stream data;
static struct system_stream keymap;

static unsigned int data_read(struct system_stream *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct kbd_interface *interface = (struct kbd_interface *)self->node.parent;
    struct base_device *device = (struct base_device *)interface->base.node.parent;
 
    return interface->read_data(device, offset, count, buffer);

}

static unsigned int data_write(struct system_stream *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct kbd_interface *interface = (struct kbd_interface *)self->node.parent;
    struct base_device *device = (struct base_device *)interface->base.node.parent;
 
    return interface->write_data(device, offset, count, buffer);

}

static unsigned int keymap_read(struct system_stream *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct kbd_interface *interface = (struct kbd_interface *)self->node.parent;
 
    return memory_read(buffer, count, interface->keymap, 256, offset);

}

static unsigned int keymap_write(struct system_stream *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct kbd_interface *interface = (struct kbd_interface *)self->node.parent;

    return memory_write(interface->keymap, 256, buffer, count, offset);

}

void kbd_register_device(struct kbd_interface *interface, struct base_device *device)
{

    system_group_add(&root, &device->module.base.node);
    system_group_add(&device->module.base, &interface->base.node);
    system_group_add(&interface->base, &data.node);
    system_group_add(&interface->base, &keymap.node);

}

void kbd_init_interface(struct kbd_interface *interface, unsigned int (*read_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct kbd_interface));
    system_init_group(&interface->base, "kbd");

    interface->read_data = read_data;
    interface->write_data = write_data;

}

void init()
{

    system_init_group(&root, "kbd");
    system_init_stream(&data, "data", data_read, data_write);
    system_init_stream(&keymap, "keymap", keymap_read, keymap_write);
    system_register_node(&root.node);

}

void destroy()
{

    system_unregister_node(&root.node);

}

