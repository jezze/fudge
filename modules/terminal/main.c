#include <fudge/module.h>
#include <base/base.h>
#include <system/system.h>
#include "terminal.h"

struct terminal_interface_group
{

    struct system_group base;
    struct terminal_interface *interface;
    struct system_stream data;

};

static struct system_group root;
static struct terminal_interface_group interfaces[32];
static unsigned int ninterfaces;

unsigned int data_read(struct system_stream *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct terminal_interface_group *group = (struct terminal_interface_group *)self->node.parent;

    return group->interface->read(group->interface, offset, count, buffer);

}

unsigned int data_write(struct system_stream *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct terminal_interface_group *group = (struct terminal_interface_group *)self->node.parent;

    return group->interface->write(group->interface, offset, count, buffer);

}

void terminal_register_interface(struct terminal_interface *interface)
{

    struct terminal_interface_group *group = &interfaces[ninterfaces];

    group->interface = interface;

    system_init_group(&group->base, interface->driver->module.name);
    system_group_add(&root, &group->base.node);
    system_init_stream(&group->data, "data", data_read, data_write);
    system_group_add(&group->base, &group->data.node);

    ninterfaces++;

}

void terminal_register_protocol(struct terminal_protocol *protocol)
{

}

void terminal_unregister_interface(struct terminal_interface *interface)
{

}

void terminal_unregister_protocol(struct terminal_protocol *protocol)
{

}

void terminal_init_interface(struct terminal_interface *interface, struct base_driver *driver, unsigned int (*read)(struct terminal_interface *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct terminal_interface *self, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct terminal_interface));

    interface->driver = driver;
    interface->read = read;
    interface->write = write;

}

void terminal_init_protocol(struct terminal_protocol *protocol, char *name)
{

    memory_clear(protocol, sizeof (struct terminal_protocol));

    protocol->name = name;

}

void init()
{

    ninterfaces = 0;

    system_init_group(&root, "terminal");
    system_register_node(&root.node);

}

void destroy()
{

    system_unregister_node(&root.node);

}

