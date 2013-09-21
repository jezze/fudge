#include <fudge/module.h>
#include <system/system.h>
#include "base.h"
#include "mouse.h"

struct mouse_group
{

    struct system_group base;
    struct base_mouse_interface *interface;
    struct base_device *device;

};

static struct system_group root;
static struct system_group dev;
static struct mouse_group groups[8];

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

static void init_group(struct mouse_group *group, struct base_mouse_interface *interface, struct base_device *device)
{

    memory_clear(group, sizeof (struct mouse_group));
    system_init_group(&group->base, device->module.name);

    group->interface = interface;
    group->device = device;

}

void base_mouse_register_interface(struct base_mouse_interface *interface, struct base_device *device)
{

    unsigned int index = find_group();

    if (!index)
        return;

    init_group(&groups[index], interface, device);
    system_group_add(&dev, &groups[index].base.node);

}

void base_mouse_init_interface(struct base_mouse_interface *interface)
{

    memory_clear(interface, sizeof (struct base_mouse_interface));

}

void base_mouse_setup()
{

    system_init_group(&root, "mouse");
    system_init_group(&dev, "dev");
    system_group_add(&root, &dev.node);
    system_register_node(&root.node);

}

