#include <fudge/module.h>
#include <system/system.h>
#include "base.h"
#include "keyboard.h"

static struct keyboard_node
{

    struct system_group base;
    struct base_keyboard_interface *interface;
    struct base_device *device;
    struct system_stream data;
    struct system_stream keymap;

} node[8];

static struct system_group root;
static struct system_group dev;

static unsigned int data_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct keyboard_node *node = (struct keyboard_node *)self->parent;
 
    return node->interface->read_data(node->device, offset, count, buffer);

}

static unsigned int keymap_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct keyboard_node *node = (struct keyboard_node *)self->parent;

    return memory_read(buffer, count, node->interface->keymap, 256, offset);

}

static unsigned int keymap_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct keyboard_node *node = (struct keyboard_node *)self->parent;

    return memory_write(node->interface->keymap, 256, buffer, count, offset);

}

static unsigned int find_node()
{

    unsigned int i;

    for (i = 1; i < 8; i++)
    {

        if (!node[i].base.node.parent)
            return i;

    }

    return 0;

}

static void init_node(struct keyboard_node *node, struct base_keyboard_interface *interface, struct base_device *device)
{

    memory_clear(node, sizeof (struct keyboard_node));
    system_init_group(&node->base, device->module.name);
    system_init_stream(&node->data, "data");
    system_init_stream(&node->keymap, "keymap");

    node->interface = interface;
    node->device = device;
    node->data.node.read = data_read;
    node->keymap.node.read = keymap_read;
    node->keymap.node.write = keymap_write;

}

void base_keyboard_register_interface(struct base_keyboard_interface *interface, struct base_device *device)
{

    unsigned int index = find_node();

    if (!index)
        return;

    init_node(&node[index], interface, device);
    system_group_add(&node[index].base, &node[index].data.node);
    system_group_add(&node[index].base, &node[index].keymap.node);
    system_group_add(&dev, &node[index].base.node);

}

void base_keyboard_init_interface(struct base_keyboard_interface *interface, unsigned int (*read_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct base_keyboard_interface));

    interface->read_data = read_data;
    interface->write_data = write_data;

}

void base_keyboard_setup()
{

    system_init_group(&root, "keyboard");
    system_init_group(&dev, "dev");
    system_group_add(&root, &dev.node);
    system_register_node(&root.node);

}

