#include <fudge/module.h>
#include <system/system.h>
#include "base.h"
#include "timer.h"

struct timer_group
{

    struct system_group base;
    struct base_timer *interface;
    struct base_device *device;

};

static struct system_group root;
static struct system_group dev;
static struct timer_group groups[8];

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

static void init_group(struct timer_group *group, struct base_timer *interface, struct base_device *device)
{

    memory_clear(group, sizeof (struct timer_group));
    system_init_group(&group->base, device->module.name);

    group->interface = interface;
    group->device = device;

}

void base_register_timer(struct base_timer *interface, struct base_device *device)
{

    unsigned int index = find_group();

    if (!index)
        return;

    init_group(&groups[index], interface, device);
    system_group_add(&dev, &groups[index].base.node);

}

void base_init_timer(struct base_timer *interface, unsigned int (*get_ticks)(struct base_device *device), void (*set_ticks)(struct base_device *device, unsigned int ticks))
{

    memory_clear(interface, sizeof (struct base_timer));

    interface->get_ticks = get_ticks;
    interface->set_ticks = set_ticks;

}

void base_setup_timer()
{

    system_init_group(&root, "timer");
    system_init_group(&dev, "dev");
    system_group_add(&root, &dev.node);
    system_register_node(&root.node);

}

