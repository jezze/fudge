#include <fudge/module.h>
#include <system/system.h>
#include "base.h"
#include "block.h"

struct block_group
{

    struct system_group base;
    struct base_block_interface *interface;
    struct base_device *device;

};

static struct system_group root;
static struct system_group dev;
static struct block_group groups[8];

static unsigned int find_group()
{

    unsigned int i;

    for (i = 1; i < 8; i++)
    {

        if (!groups[i].base.node.parent)
            return i;

    }

    return 0;

}

static void init_group(struct block_group *group, struct base_block_interface *interface, struct base_device *device)
{

    memory_clear(group, sizeof (struct block_group));
    system_init_group(&group->base, device->module.name);

    group->interface = interface;
    group->device = device;

}

void base_block_register_interface(struct base_block_interface *interface, struct base_device *device)
{

    unsigned int index = find_group();

    if (!index)
        return;

    init_group(&groups[index], interface, device);
    system_group_add(&dev, &groups[index].base.node);

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

