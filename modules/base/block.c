#include <fudge/module.h>
#include <system/system.h>
#include "base.h"
#include "block.h"

struct block_device_node
{

    struct system_group base;
    struct base_block_interface *interface;
    struct base_device *device;

};

static struct system_group root;
static struct system_group dev;
static struct block_device_node dnodes[8];

static unsigned int find_device_node()
{

    unsigned int i;

    for (i = 1; i < 8; i++)
    {

        if (!dnodes[i].base.node.parent)
            return i;

    }

    return 0;

}

static void init_device_node(struct block_device_node *node, struct base_block_interface *interface, struct base_device *device)
{

    memory_clear(node, sizeof (struct block_device_node));
    system_init_group(&node->base, device->module.name);

    node->interface = interface;
    node->device = device;

}

void base_block_register_interface(struct base_block_interface *interface, struct base_device *device)
{

    unsigned int index = find_device_node();

    if (!index)
        return;

    init_device_node(&dnodes[index], interface, device);
    system_group_add(&dev, &dnodes[index].base.node);

}

void base_block_register_protocol(struct base_block_protocol *protocol)
{

}

void base_block_init_interface(struct base_block_interface *interface, unsigned int (*read)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct base_block_interface));

    interface->read = read;
    interface->write = write;

}

void base_block_init_protocol(struct base_block_protocol *protocol, char *name)
{

    memory_clear(protocol, sizeof (struct base_block_protocol));

    protocol->name = name;

}

void base_block_setup()
{

    system_init_group(&root, "block");
    system_init_group(&dev, "dev");
    system_group_add(&root, &dev.node);
    system_register_node(&root.node);

}

