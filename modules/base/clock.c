#include <fudge/module.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include "base.h"
#include "clock.h"

static struct clock_node
{

    struct system_group base;
    struct base_clock_interface *interface;
    struct base_device *device;
    struct system_stream timestamp;

} node[8];

static struct system_group root;
static struct system_group dev;

static unsigned int timestamp_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct clock_node *node = (struct clock_node *)self->parent;
    char num[32];

    return memory_read(buffer, count, num, memory_write_number(num, 32, node->interface->timestamp, 10, 0), offset);

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

static void init_node(struct clock_node *node, struct base_clock_interface *interface, struct base_device *device)
{

    memory_clear(node, sizeof (struct clock_node));
    system_init_group(&node->base, device->module.name);
    system_init_stream(&node->timestamp, "timestamp");

    node->interface = interface;
    node->device = device;
    node->timestamp.node.read = timestamp_read;

}

void base_clock_register_interface(struct base_clock_interface *interface, struct base_device *device)
{

    unsigned int index = find_node();

    if (!index)
        return;

    init_node(&node[index], interface, device);
    system_group_add(&dev, &node[index].base.node);
    system_group_add(&node[index].base, &node[index].timestamp.node);

}

void base_clock_init_interface(struct base_clock_interface *interface)
{

    memory_clear(interface, sizeof (struct base_clock_interface));

}

void base_clock_setup()
{

    memory_clear(node, sizeof (struct clock_node) * 8);
    system_init_group(&root, "clock");
    system_register_node(&root.node);
    system_init_group(&dev, "dev");
    system_group_add(&root, &dev.node);

}

