#include <fudge/memory.h>
#include <base/base.h>
#include <system/system.h>
#include "mouse.h"

static struct system_group root;

void mouse_register_interface(struct mouse_interface *interface)
{

    system_group_add(&root, &interface->node.root.base);
    system_group_add(&interface->node.root, &interface->node.x.base);
    system_group_add(&interface->node.root, &interface->node.y.base);

}

void mouse_register_protocol(struct mouse_protocol *protocol)
{

}

void mouse_unregister_interface(struct mouse_interface *interface)
{

    system_group_remove(&interface->node.root, &interface->node.x.base);
    system_group_remove(&interface->node.root, &interface->node.y.base);
    system_group_remove(&root, &interface->node.root.base);

}

void mouse_unregister_protocol(struct mouse_protocol *protocol)
{

}

void mouse_init_interface(struct mouse_interface *interface, struct base_driver *driver)
{

    memory_clear(interface, sizeof (struct mouse_interface));

    interface->driver = driver;

    system_init_group(&interface->node.root, interface->driver->module.name);
    system_init_integer(&interface->node.x, "x", interface->x);
    system_init_integer(&interface->node.y, "y", interface->y);

}

void mouse_init_protocol(struct mouse_protocol *protocol, char *name)
{

    memory_clear(protocol, sizeof (struct mouse_protocol));

    protocol->name = name;

}

void init()
{

    system_init_group(&root, "mouse");
    system_register_node(&root.base);

}

void destroy()
{

    system_unregister_node(&root.base);

}

