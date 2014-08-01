#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include "base.h"
#include "keyboard.h"

static struct interface_node
{

    struct system_group base;
    struct system_stream data;
    struct system_stream keymap;
    struct base_device device;

} inode[8];

static struct system_group root;

static unsigned int data_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct interface_node *node = (struct interface_node *)self->parent;
    struct base_keyboard_interface *interface = (struct base_keyboard_interface *)node->device.interface;

    return interface->read_data(node->device.bus, node->device.id, offset, count, buffer);

}

static unsigned int keymap_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct interface_node *node = (struct interface_node *)self->parent;
    struct base_keyboard_interface *interface = (struct base_keyboard_interface *)node->device.interface;

    return interface->read_keymap(node->device.bus, node->device.id, offset, count, buffer);

}

static unsigned int keymap_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct interface_node *node = (struct interface_node *)self->parent;
    struct base_keyboard_interface *interface = (struct base_keyboard_interface *)node->device.interface;

    return interface->write_keymap(node->device.bus, node->device.id, offset, count, buffer);

}

static unsigned int find_inode()
{

    unsigned int i;

    for (i = 1; i < 8; i++)
    {

        if (!inode[i].base.node.parent)
            return i;

    }

    return 0;

}

static void init_inode(struct interface_node *node, struct base_interface *interface, struct base_bus *bus, unsigned int id)
{

    memory_clear(node, sizeof (struct interface_node));
    system_init_group(&node->base, bus->name);
    system_init_stream(&node->data, "data");
    system_init_stream(&node->keymap, "keymap");
    base_init_device(&node->device, interface, bus, id);

    node->data.node.read = data_read;
    node->keymap.node.read = keymap_read;
    node->keymap.node.write = keymap_write;

}

void base_keyboard_connect_interface(struct base_interface *interface, struct base_bus *bus, unsigned int id)
{

    unsigned int index = find_inode();

    if (!index)
        return;

    init_inode(&inode[index], interface, bus, id);
    system_group_add(&root, &inode[index].base.node);
    system_group_add(&inode[index].base, &inode[index].data.node);
    system_group_add(&inode[index].base, &inode[index].keymap.node);

}

void base_keyboard_register_interface(struct base_keyboard_interface *interface)
{

    base_register_interface(&interface->base);

}

void base_keyboard_unregister_interface(struct base_keyboard_interface *interface)
{

    base_unregister_interface(&interface->base);

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

void base_keyboard_setup()
{

    memory_clear(inode, sizeof (struct interface_node) * 8);
    system_init_group(&root, "keyboard");
    system_register_node(&root.node);

}

