#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <kernel/rendezvous.h>
#include <system/system.h>
#include "base.h"
#include "keyboard.h"

static struct interface_node
{

    struct system_group base;
    struct base_keyboard_interface *interface;
    struct base_device *device;
    struct system_stream data;
    struct system_stream keymap;

} inode[8];

static struct system_group root;
static struct system_group dev;

static unsigned int data_open(struct system_node *self)
{

    struct interface_node *node = (struct interface_node *)self->parent;

    rendezvous_lock(&node->interface->rdata);

    return (unsigned int)self;

}

static unsigned int data_close(struct system_node *self)
{

    struct interface_node *node = (struct interface_node *)self->parent;

    rendezvous_unlock(&node->interface->rdata);

    return (unsigned int)self;

}

static unsigned int data_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct interface_node *node = (struct interface_node *)self->parent;

    return node->interface->read_data(node->device, offset, count, buffer);

}

static unsigned int keymap_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct interface_node *node = (struct interface_node *)self->parent;

    return memory_read(buffer, count, node->interface->keymap, 2048, offset);

}

static unsigned int keymap_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct interface_node *node = (struct interface_node *)self->parent;

    return memory_write(node->interface->keymap, 2048, buffer, count, offset);

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

static void init_inode(struct interface_node *node, struct base_keyboard_interface *interface, struct base_device *device)
{

    memory_clear(node, sizeof (struct interface_node));
    system_init_group(&node->base, device->module.name);
    system_init_stream(&node->data, "data");
    system_init_stream(&node->keymap, "keymap");

    node->interface = interface;
    node->device = device;
    node->data.node.open = data_open;
    node->data.node.close = data_close;
    node->data.node.read = data_read;
    node->keymap.node.read = keymap_read;
    node->keymap.node.write = keymap_write;

}

void base_keyboard_register_interface(struct base_keyboard_interface *interface, struct base_device *device)
{

    unsigned int index = find_inode();

    if (!index)
        return;

    init_inode(&inode[index], interface, device);
    system_group_add(&dev, &inode[index].base.node);
    system_group_add(&inode[index].base, &inode[index].data.node);
    system_group_add(&inode[index].base, &inode[index].keymap.node);

}

void base_keyboard_init_interface(struct base_keyboard_interface *interface, unsigned int (*read_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct base_keyboard_interface));
    base_init_interface(&interface->base, BASE_INTERFACE_TYPE_OTHER);

    interface->read_data = read_data;
    interface->write_data = write_data;

}

void base_keyboard_setup()
{

    memory_clear(inode, sizeof (struct interface_node) * 8);
    system_init_group(&root, "keyboard");
    system_register_node(&root.node);
    system_init_group(&dev, "dev");
    system_group_add(&root, &dev.node);

}

