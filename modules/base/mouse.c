#include <fudge/module.h>
#include <system/system.h>
#include "base.h"
#include "mouse.h"

static struct system_group root;

void base_register_mouse(struct base_mouse *interface, struct base_device *device)
{

    system_group_add(&root, &device->module.base.node);
    system_group_add(&device->module.base, &interface->base.node);

}

void base_init_mouse(struct base_mouse *interface)
{

    memory_clear(interface, sizeof (struct base_mouse));
    system_init_group(&interface->base, "mouse");

}

void base_setup_mouse()
{

    system_init_group(&root, "mouse");
    system_register_node(&root.node);

}

