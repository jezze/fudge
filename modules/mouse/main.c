#include <fudge/module.h>
#include <system/system.h>
#include <base/base.h>
#include "mouse.h"

static struct system_group root;

void mouse_register_device(struct mouse_interface *interface, struct base_device *device)
{

    system_group_add(&root, &device->module.base.node);
    system_group_add(&device->module.base, &interface->base.node);

}

void mouse_init_interface(struct mouse_interface *interface)
{

    memory_clear(interface, sizeof (struct mouse_interface));
    system_init_group(&interface->base, "mouse");

}

void init()
{

    system_init_group(&root, "mouse");
    system_register_node(&root.node);

}

void destroy()
{

    system_unregister_node(&root.node);

}

