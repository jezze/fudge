#include <fudge/module.h>
#include <system/system.h>
#include "base.h"
#include "timer.h"

struct timer_group
{

    struct system_group base;
    struct system_stream control;
    struct system_stream ticks;
    char name[8];
    struct modules_device *device;

};

static struct system_group timer;
static struct system_stream clone;
static unsigned int count;
static struct timer_group groups[8];

static unsigned int control_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct timer_group *group = (struct timer_group *)self->parent;

    return memory_read(buffer, count, group->name, string_length(group->name), offset);

}

static unsigned int ticks_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    return memory_read(buffer, count, "ticks", 5, offset);

}

static unsigned int clone_open(struct system_node *self)
{

    struct timer_group *current = &groups[++count];

    memory_clear(current->name, 8);
    memory_write_number(current->name, 8, count, 10, 0);

    system_init_group(&current->base, current->name);
    system_group_add(&timer, &current->base.node);

    system_init_stream(&current->control, "control");
    system_group_add(&current->base, &current->control.node);

    current->control.node.read = control_read;

    system_init_stream(&current->ticks, "ticks");
    system_group_add(&current->base, &current->ticks.node);

    current->ticks.node.read = ticks_read;

    return (unsigned int)&current->control;

}

void base_init_timer(struct base_timer *interface, unsigned int (*get_ticks)(struct base_device *device), void (*set_ticks)(struct base_device *device, unsigned int ticks))
{

    memory_clear(interface, sizeof (struct base_timer));

    interface->get_ticks = get_ticks;
    interface->set_ticks = set_ticks;

}

void base_setup_timer(struct system_group *root)
{

    count = 0;

    system_init_group(&timer, "timer");
    system_group_add(root, &timer.node);
    system_init_stream(&clone, "clone");
    system_group_add(&timer, &clone.node);

    clone.node.open = clone_open;

}

