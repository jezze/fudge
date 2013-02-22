#include <fudge/module.h>
#include <base/base.h>
#include <system/system.h>
#include "mouse.h"

static struct system_group root;

void mouse_register_interface(struct mouse_interface *interface)
{

}

void mouse_register_protocol(struct mouse_protocol *protocol)
{

}

void mouse_unregister_interface(struct mouse_interface *interface)
{

}

void mouse_unregister_protocol(struct mouse_protocol *protocol)
{

}

void mouse_init_interface(struct mouse_interface *interface, struct base_driver *driver)
{

    memory_clear(interface, sizeof (struct mouse_interface));

    interface->driver = driver;

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

