#include <fudge/module.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include "base.h"
#include "timer.h"

static struct interface_node
{

    struct system_group base;
    struct base_timer_interface *interface;
    struct base_device *device;
    struct system_stream jiffies;

} inode[8];

static struct session_node
{

    struct system_group base;
    struct interface_node *inode;
    struct system_group device;
    struct system_stream control;
    struct system_stream sleep;
    char name[8];

} snode[8];

static struct system_group root;
static struct system_group dev;
static struct system_stream clone;

static unsigned int sleep_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct session_node *node = (struct session_node *)self->parent;

    node->inode->interface->add_duration(node->inode->device, 3);

    return 0;

}

static unsigned int control_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct session_node *node = (struct session_node *)self->parent;

    return memory_read(buffer, count, node->name, ascii_length(node->name), offset);

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
    system_init_stream(&node->sleep, "sleep");

    node->inode = inode;
    node->device.node.open = device_open;
    node->device.node.walk = device_walk;
    node->control.node.read = control_read;
    node->sleep.node.read = sleep_read;

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
    system_group_add(&snode[index].base, &snode[index].sleep.node);

    return (unsigned int)&snode[index].control;

}

static unsigned int jiffies_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct interface_node *node = (struct interface_node *)self->parent;
    char num[32];

    return memory_read(buffer, count, num, ascii_write_value(num, 32, node->interface->jiffies, 10, 0), offset);

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

static void init_inode(struct interface_node *node, struct base_timer_interface *interface, struct base_device *device)
{

    memory_clear(node, sizeof (struct interface_node));
    system_init_group(&node->base, device->module.name);
    system_init_stream(&node->jiffies, "jiffies");

    node->interface = interface;
    node->device = device;
    node->jiffies.node.read = jiffies_read;

}

void base_timer_register_interface(struct base_timer_interface *interface, struct base_device *device)
{

    unsigned int index = find_inode();

    if (!index)
        return;

    init_inode(&inode[index], interface, device);
    system_group_add(&dev, &inode[index].base.node);
    system_group_add(&inode[index].base, &inode[index].jiffies.node);

}

void base_timer_init_interface(struct base_timer_interface *interface, void (*add_duration)(struct base_device *device, unsigned int duration))
{

    memory_clear(interface, sizeof (struct base_timer_interface));

    interface->add_duration = add_duration;

}

void base_timer_setup()
{

    memory_clear(inode, sizeof (struct interface_node) * 8);
    memory_clear(snode, sizeof (struct session_node) * 8);
    system_init_group(&root, "timer");
    system_register_node(&root.node);
    system_init_group(&dev, "dev");
    system_group_add(&root, &dev.node);
    system_init_stream(&clone, "clone");
    system_group_add(&root, &clone.node);

    clone.node.open = clone_open;

}

