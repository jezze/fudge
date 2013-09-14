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
static struct timer_group groups[8];

static unsigned int control_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct timer_group *group = (struct timer_group *)self->parent;

    return memory_read(buffer, count, group->name, string_length(group->name), offset);

}

static unsigned int control_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct timer_group *group = (struct timer_group *)self->parent;
    struct system_group *root = (struct system_group *)self->parent->parent;

    if (memory_match(buffer, "close", 5))
        system_group_remove(root, &group->base.node);

    return 0;

}

static unsigned int ticks_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    return memory_read(buffer, count, "ticks", 5, offset);

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

static void init_group(struct timer_group *group, unsigned int id)
{

    memory_clear(group, sizeof (struct timer_group));

    memory_write_number(group->name, 8, id, 10, 0);
    system_init_group(&group->base, group->name);
    system_init_stream(&group->control, "control");
    system_group_add(&group->base, &group->control.node);
    system_init_stream(&group->ticks, "ticks");
    system_group_add(&group->base, &group->ticks.node);

    group->control.node.read = control_read;
    group->control.node.write = control_write;
    group->ticks.node.read = ticks_read;

}

static unsigned int clone_open(struct system_node *self)
{

    unsigned int index = find_group();

    if (!index)
        return 0;

    init_group(&groups[index], index);
    system_group_add(&timer, &groups[index].base.node);

    return (unsigned int)&groups[index].control;

}

void base_init_timer(struct base_timer *interface, unsigned int (*get_ticks)(struct base_device *device), void (*set_ticks)(struct base_device *device, unsigned int ticks))
{

    memory_clear(interface, sizeof (struct base_timer));

    interface->get_ticks = get_ticks;
    interface->set_ticks = set_ticks;

}

void base_setup_timer(struct system_group *root)
{

    system_init_group(&timer, "timer");
    system_group_add(root, &timer.node);
    system_init_stream(&clone, "clone");
    system_group_add(&timer, &clone.node);

    clone.node.open = clone_open;

}

