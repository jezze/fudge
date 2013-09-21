#include <fudge/module.h>
#include <system/system.h>
#include "base.h"
#include "timer.h"

struct timer_node
{

    struct system_group base;
    struct base_timer_interface *interface;
    struct base_device *device;

};

static struct system_group root;
static struct system_group dev;
static struct timer_node node[8];

static unsigned int find_node()
{

    unsigned int i;

    for (i = 1; i < 8; i++)
    {

        if (!node[i].base.node.parent)
            return i;

    }

    return 0;

}

static void init_node(struct timer_node *node, struct base_timer_interface *interface, struct base_device *device)
{

    memory_clear(node, sizeof (struct timer_node));
    system_init_group(&node->base, device->module.name);

    node->interface = interface;
    node->device = device;

}

void base_timer_register_interface(struct base_timer_interface *interface, struct base_device *device)
{

    unsigned int index = find_node();

    if (!index)
        return;

    init_node(&node[index], interface, device);
    system_group_add(&dev, &node[index].base.node);

}

void base_timer_init_interface(struct base_timer_interface *interface, unsigned int (*get_ticks)(struct base_device *device), void (*set_ticks)(struct base_device *device, unsigned int ticks))
{

    memory_clear(interface, sizeof (struct base_timer_interface));

    interface->get_ticks = get_ticks;
    interface->set_ticks = set_ticks;

}

void base_timer_setup()
{

    system_init_group(&root, "timer");
    system_init_group(&dev, "dev");
    system_group_add(&root, &dev.node);
    system_register_node(&root.node);

}

