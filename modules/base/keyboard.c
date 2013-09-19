#include <fudge/module.h>
#include <system/system.h>
#include "base.h"
#include "keyboard.h"

struct keyboard_group
{

    struct system_group base;
    struct base_keyboard *interface;
    struct base_device *device;
    struct system_stream data;
    struct system_stream keymap;

};

static struct system_group root;
static struct system_group dev;
static struct keyboard_group groups[8];

static unsigned int data_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct keyboard_group *group = (struct keyboard_group *)self->parent;
 
    return group->interface->read_data(group->device, offset, count, buffer);

}

static unsigned int keymap_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct keyboard_group *group = (struct keyboard_group *)self->parent;

    return memory_read(buffer, count, group->interface->keymap, 256, offset);

}

static unsigned int keymap_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct keyboard_group *group = (struct keyboard_group *)self->parent;

    return memory_write(group->interface->keymap, 256, buffer, count, offset);

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

static void init_group(struct keyboard_group *group, struct base_keyboard *interface, struct base_device *device)
{

    memory_clear(group, sizeof (struct keyboard_group));
    system_init_group(&group->base, device->module.name);
    system_init_stream(&group->data, "data");
    system_init_stream(&group->keymap, "keymap");

    group->interface = interface;
    group->device = device;

    group->data.node.read = data_read;
    group->keymap.node.read = keymap_read;
    group->keymap.node.write = keymap_write;

}

void base_register_keyboard(struct base_keyboard *interface, struct base_device *device)
{

    unsigned int index = find_group();

    if (!index)
        return;

    init_group(&groups[index], interface, device);
    system_group_add(&groups[index].base, &groups[index].data.node);
    system_group_add(&groups[index].base, &groups[index].keymap.node);
    system_group_add(&dev, &groups[index].base.node);

}

void base_init_keyboard(struct base_keyboard *interface, unsigned int (*read_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct base_keyboard));

    interface->read_data = read_data;
    interface->write_data = write_data;

}

void base_setup_keyboard()
{

    system_init_group(&root, "keyboard");
    system_init_group(&dev, "dev");
    system_group_add(&root, &dev.node);
    system_register_node(&root.node);

}

