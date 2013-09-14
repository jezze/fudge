#include <fudge/module.h>
#include <system/system.h>
#include "base.h"
#include "timer.h"

static struct system_group timer;
static struct system_stream clone;

static struct
{

    unsigned int count;
    struct {struct system_group index; struct system_stream control; struct system_stream ticks; struct modules_device *device;} items[8];

} timers;

static unsigned int control_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    return memory_read(buffer, count, "control", 7, offset);

}

static unsigned int ticks_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    return memory_read(buffer, count, "ticks", 5, offset);

}

static unsigned int clone_open(struct system_node *self)
{

    timers.count++;

    system_init_group(&timers.items[timers.count].index, "0");
    system_group_add(&timer, &timers.items[timers.count].index.node);
    system_init_stream(&timers.items[timers.count].control, "control");
    system_group_add(&timers.items[timers.count].index, &timers.items[timers.count].control.node);

    timers.items[timers.count].control.node.read = control_read;

    system_init_stream(&timers.items[timers.count].ticks, "ticks");
    system_group_add(&timers.items[timers.count].index, &timers.items[timers.count].ticks.node);

    timers.items[timers.count].ticks.node.read = ticks_read;

    return (unsigned int)&timers.items[timers.count].control;

}

void base_init_timer(struct base_timer *interface, unsigned int (*get_ticks)(struct base_device *device), void (*set_ticks)(struct base_device *device, unsigned int ticks))
{

    memory_clear(interface, sizeof (struct base_timer));

    interface->get_ticks = get_ticks;
    interface->set_ticks = set_ticks;

}

void base_setup_timer(struct system_group *root)
{

    timers.count = 0;

    system_init_group(&timer, "timer");
    system_group_add(root, &timer.node);
    system_init_stream(&clone, "clone");
    system_group_add(&timer, &clone.node);

    clone.node.open = clone_open;

}

