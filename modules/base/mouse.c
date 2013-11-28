#include <fudge/module.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include "base.h"
#include "mouse.h"

static struct interface_node
{

    struct system_group base;
    struct base_mouse_interface *interface;
    struct base_device *device;
    struct system_stream data;
    struct system_stream vx;
    struct system_stream vy;

} inode[8];

static struct session_node
{

    struct system_group base;
    struct interface_node *inode;
    struct system_group device;
    struct system_stream control;
    char name[8];

} snode[8];

static struct system_group root;
static struct system_group dev;
static struct system_stream clone;

static unsigned int control_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct session_node *node = (struct session_node *)self->parent;

    return memory_read(buffer, count, node->name, string_length(node->name), offset);

}

static unsigned int device_open(struct system_node *self)
{

    struct session_node *node = (struct session_node *)self->parent;

    return node->inode->base.node.open(&node->inode->base.node);

}

static unsigned int device_walk(struct system_node *self, unsigned int count, const char *path)
{

    struct session_node *node = (struct session_node *)self->parent;

    return node->inode->base.node.walk(&node->inode->base.node, count, path);

}

static unsigned int find_snode()
{

    unsigned int i;

    for (i = 1; i < 8; i++)
    {

        if (!snode[i].base.node.parent)
            return i;

    }

    return 0;

}

static void init_snode(struct session_node *node, unsigned int id, struct interface_node *inode)
{

    memory_clear(node, sizeof (struct session_node));
    ascii_fromint(node->name, 8, id, 10);
    system_init_group(&node->base, node->name);
    system_init_group(&node->device, "device");
    system_init_stream(&node->control, "control");

    node->inode = inode;
    node->device.node.open = device_open;
    node->device.node.walk = device_walk;
    node->control.node.read = control_read;

}

static unsigned int clone_open(struct system_node *self)
{

    unsigned int index = find_snode();

    if (!index)
        return 0;

    init_snode(&snode[index], index, &inode[1]);
    system_group_add(&root, &snode[index].base.node);
    system_group_add(&snode[index].base, &snode[index].device.node);
    system_group_add(&snode[index].base, &snode[index].control.node);

    return (unsigned int)&snode[index].control;

}

static unsigned int data_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct interface_node *node = (struct interface_node *)self->parent;

    return node->interface->read_data(node->device, offset, count, buffer);

}

static unsigned int vx_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct interface_node *node = (struct interface_node *)self->parent;
    unsigned char num[32];

    return memory_read(buffer, count, num, ascii_write_value(num, 32, node->interface->vx, 10, 0), offset);

}

static unsigned int vy_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct interface_node *node = (struct interface_node *)self->parent;
    unsigned char num[32];

    return memory_read(buffer, count, num, ascii_write_value(num, 32, node->interface->vy, 10, 0), offset);

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

static void init_inode(struct interface_node *node, struct base_mouse_interface *interface, struct base_device *device)
{

    memory_clear(node, sizeof (struct interface_node));
    system_init_group(&node->base, device->module.name);
    system_init_stream(&node->data, "data");
    system_init_stream(&node->vx, "vx");
    system_init_stream(&node->vy, "vy");

    node->interface = interface;
    node->device = device;
    node->data.node.read = data_read;
    node->vx.node.read = vx_read;
    node->vy.node.read = vy_read;

}

void base_mouse_register_interface(struct base_mouse_interface *interface, struct base_device *device)
{

    unsigned int index = find_inode();

    if (!index)
        return;

    init_inode(&inode[index], interface, device);
    system_group_add(&dev, &inode[index].base.node);
    system_group_add(&inode[index].base, &inode[index].data.node);
    system_group_add(&inode[index].base, &inode[index].vx.node);
    system_group_add(&inode[index].base, &inode[index].vy.node);

}

void base_mouse_init_interface(struct base_mouse_interface *interface, unsigned int (*read_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct base_mouse_interface));

    interface->read_data = read_data;

}

void base_mouse_setup()
{

    memory_clear(inode, sizeof (struct interface_node) * 8);
    memory_clear(snode, sizeof (struct session_node) * 8);
    system_init_group(&root, "mouse");
    system_register_node(&root.node);
    system_init_group(&dev, "dev");
    system_group_add(&root, &dev.node);
    system_init_stream(&clone, "clone");
    system_group_add(&root, &clone.node);

    clone.node.open = clone_open;

}

