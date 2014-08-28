#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include "base.h"
#include "keyboard.h"

static struct system_node root;

static unsigned int data_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_keyboard_node *node = (struct base_keyboard_node *)self->parent;

    return node->interface->read_data(node->device->bus, node->device->id, offset, count, buffer);

}

static unsigned int keymap_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_keyboard_node *node = (struct base_keyboard_node *)self->parent;

    return node->interface->read_keymap(node->device->bus, node->device->id, offset, count, buffer);

}

static unsigned int keymap_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_keyboard_node *node = (struct base_keyboard_node *)self->parent;

    return node->interface->write_keymap(node->device->bus, node->device->id, offset, count, buffer);

}

void base_keyboard_register_interface(struct base_keyboard_interface *interface)
{

    base_register_interface(&interface->base);

}

void base_keyboard_register_node(struct base_keyboard_node *node)
{

    system_add_child(&root, &node->base);
    system_add_child(&node->base, &node->data);
    system_add_child(&node->base, &node->keymap);

}

void base_keyboard_unregister_interface(struct base_keyboard_interface *interface)
{

    base_unregister_interface(&interface->base);

}

void base_keyboard_unregister_node(struct base_keyboard_node *node)
{

}

void base_keyboard_init_interface(struct base_keyboard_interface *interface, unsigned int (*read_data)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_data)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*read_keymap)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_keymap)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct base_keyboard_interface));
    base_init_interface(&interface->base, BASE_INTERFACE_TYPE_KEYBOARD);

    interface->read_data = read_data;
    interface->write_data = write_data;
    interface->read_keymap = read_keymap;
    interface->write_keymap = write_keymap;

}

void base_keyboard_init_node(struct base_keyboard_node *node, struct base_device *device, struct base_keyboard_interface *interface)
{

    memory_clear(node, sizeof (struct base_keyboard_node));
    system_init_multigroup(&node->base, device->bus->name);
    system_init_stream(&node->data, "data");
    system_init_stream(&node->keymap, "keymap");

    node->device = device;
    node->interface = interface;
    node->data.read = data_read;
    node->keymap.read = keymap_read;
    node->keymap.write = keymap_write;

}

void base_keyboard_setup()
{

    system_init_group(&root, "keyboard");
    system_register_node(&root);

}

