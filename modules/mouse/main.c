#include <fudge/module.h>
#include <system/system.h>
#include <base/base.h>
#include "mouse.h"

static struct system_group root;

void mouse_register_interface(struct mouse_interface *interface)
{

    system_group_add(&root, &interface->node.root.node);
    system_group_add(&interface->node.root, &interface->node.vx.node);
    system_group_add(&interface->node.root, &interface->node.vy.node);

}

void mouse_register_protocol(struct mouse_protocol *protocol)
{

}

void mouse_unregister_interface(struct mouse_interface *interface)
{

    system_group_remove(&interface->node.root, &interface->node.vx.node);
    system_group_remove(&interface->node.root, &interface->node.vy.node);
    system_group_remove(&root, &interface->node.root.node);

}

void mouse_unregister_protocol(struct mouse_protocol *protocol)
{

}

void mouse_init_interface(struct mouse_interface *interface, struct base_driver *driver)
{

    memory_clear(interface, sizeof (struct mouse_interface));

    interface->driver = driver;

    system_init_group(&interface->node.root, interface->driver->module.name);
    system_init_integer(&interface->node.vx, "vx", (int *)&interface->vx);
    system_init_integer(&interface->node.vy, "vy", (int *)&interface->vy);

}

void mouse_init_protocol(struct mouse_protocol *protocol, char *name)
{

    memory_clear(protocol, sizeof (struct mouse_protocol));

    protocol->name = name;

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

