#include <fudge/module.h>
#include <system/system.h>
#include "base.h"
#include "terminal.h"

struct terminal_group
{

    struct system_group base;
    struct base_terminal_interface *interface;
    struct base_device *device;
    struct system_stream data;

};

static struct system_group root;
static struct system_group dev;
static struct terminal_group groups[8];

static unsigned int data_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct terminal_group *group = (struct terminal_group *)self->parent;
 
    return group->interface->read_data(group->device, offset, count, buffer);

}

static unsigned int data_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct terminal_group *group = (struct terminal_group *)self->parent;

    return group->interface->write_data(group->device, offset, count, buffer);

}

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

static void init_group(struct terminal_group *group, struct base_terminal_interface *interface, struct base_device *device)
{

    memory_clear(group, sizeof (struct terminal_group));
    system_init_group(&group->base, device->module.name);
    system_init_stream(&group->data, "data");

    group->interface = interface;
    group->device = device;

    group->data.node.read = data_read;
    group->data.node.write = data_write;

}

void base_terminal_register_interface(struct base_terminal_interface *interface, struct base_device *device)
{

    unsigned int index = find_group();

    if (!index)
        return;

    init_group(&groups[index], interface, device);
    system_group_add(&groups[index].base, &groups[index].data.node);
    system_group_add(&dev, &groups[index].base.node);

}

void base_terminal_init_interface(struct base_terminal_interface *interface, unsigned int (*read_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct base_terminal_interface));

    interface->read_data = read_data;
    interface->write_data = write_data;

}

void base_terminal_setup()
{

    system_init_group(&root, "terminal");
    system_init_group(&dev, "dev");
    system_group_add(&root, &dev.node);
    system_register_node(&root.node);

}

