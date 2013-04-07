#include <fudge/module.h>
#include <base/base.h>
#include <system/system.h>
#include "mouse.h"

struct mouse_interface_group
{

    struct system_group base;
    struct mouse_interface *interface;
    struct system_stream data;

};

static struct system_group root;
static struct mouse_interface_group interfaces[32];
static unsigned int ninterfaces;

unsigned int data_read(struct system_stream *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

unsigned int data_write(struct system_stream *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

void mouse_register_interface(struct mouse_interface *interface)
{

    struct mouse_interface_group *group = &interfaces[ninterfaces];

    group->interface = interface;

    system_init_group(&group->base, interface->driver->module.name);
    system_group_add(&root, &group->base.node);
    system_init_stream(&group->data, "data", data_read, data_write);
    system_group_add(&group->base, &group->data.node);

    ninterfaces++;

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

    ninterfaces = 0;

    system_init_group(&root, "mouse");
    system_register_node(&root.node);

}

void destroy()
{

    system_unregister_node(&root.node);

}

