#include <fudge/module.h>
#include <system/system.h>
#include <base/base.h>
#include "timer.h"

static struct system_group root;
static struct system_stream ticks;

unsigned int ticks_read(struct system_stream *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct timer_interface *interface = (struct timer_interface *)self;
    struct base_device_node *node = (struct base_device_node *)self->node.parent;
    struct base_device *device = node->device;
    char num[32];

    return memory_read(buffer, count, num, memory_write_number(num, 32, interface->get_ticks(device), 10, 0), offset);

}

unsigned int ticks_write(struct system_stream *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

void timer_register_device(struct timer_interface *itimer, struct base_device *device)
{

    base_init_node(device);
    system_group_add(&root, &device->node.base.node);
    system_group_add(&device->node.base, &itimer->base.node);
    system_group_add(&itimer->base, &ticks.node);

}

void timer_init_interface(struct timer_interface *itimer, unsigned int (*get_ticks)(struct base_device *device), void (*set_ticks)(struct base_device *device, unsigned int ticks))
{

    memory_clear(itimer, sizeof (struct timer_interface));
    system_init_group(&itimer->base, "timer");

    itimer->get_ticks = get_ticks;
    itimer->set_ticks = set_ticks;

}

void init()
{

    system_init_group(&root, "timer");
    system_init_stream(&ticks, "ticks", ticks_read, ticks_write);
    system_register_node(&root.node);

}

void destroy()
{

    system_unregister_node(&root.node);

}

