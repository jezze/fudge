#include <fudge/module.h>
#include <system/system.h>
#include "base.h"
#include "block.h"

static struct system_group root;

void base_register_block(struct base_block *interface, struct base_device *device)
{

    system_group_add(&root, &device->module.base.node);
    system_group_add(&device->module.base, &interface->base.node);

}

void base_init_block(struct base_block *interface, unsigned int (*read)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct base_block));
    system_init_group(&interface->base, "block");

    interface->read = read;
    interface->write = write;

}

void base_setup_block()
{

    system_init_group(&root, "block");
    system_register_node(&root.node);

}

