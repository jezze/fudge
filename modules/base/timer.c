#include <fudge/module.h>
#include <system/system.h>
#include "base.h"
#include "timer.h"

static struct system_group timer;
static struct system_stream clone;
static unsigned int ntimers;

static struct
{

    struct system_group index;
    struct system_stream control;
    struct system_stream ticks;
    struct modules_device *device;

} timers[8];

static unsigned int clone_open(struct system_node *self)
{

    return (unsigned int)self;

}

static unsigned int clone_close(struct system_node *self)
{

    return (unsigned int)self;

}

static unsigned int clone_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int clone_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    ntimers++;

    system_init_group(&timers[ntimers].index, "0");
    system_group_add(&timer, &timers[ntimers].index.node);

    return ntimers;

}

void base_init_timer(struct base_timer *interface, unsigned int (*get_ticks)(struct base_device *device), void (*set_ticks)(struct base_device *device, unsigned int ticks))
{

    memory_clear(interface, sizeof (struct base_timer));

    interface->get_ticks = get_ticks;
    interface->set_ticks = set_ticks;

}

void base_setup_timer(struct system_group *root)
{

    ntimers = 0;

    system_init_group(&timer, "timer");
    system_group_add(root, &timer.node);
    system_init_stream(&clone, "clone", clone_open, clone_close, clone_read, clone_write);
    system_group_add(&timer, &clone.node);

}

