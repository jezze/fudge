#include <fudge/module.h>
#include <base/base.h>
#include <system/system.h>
#include "kbd.h"

struct kbd_interface_group
{

    struct system_group base;
    struct kbd_interface *interface;
    struct system_stream data;

};

static struct system_group root;
static struct kbd_interface_group interfaces[32];
static unsigned int ninterfaces;

unsigned int data_read(struct system_stream *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct kbd_interface_group *group = (struct kbd_interface_group *)self->node.parent;

    return group->interface->read_data(group->interface, offset, count, buffer);

}

unsigned int data_write(struct system_stream *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct kbd_interface_group *group = (struct kbd_interface_group *)self->node.parent;

    return group->interface->write_data(group->interface, offset, count, buffer);

}

void kbd_register_interface(struct kbd_interface *interface)
{

    struct kbd_interface_group *group = &interfaces[ninterfaces];

    group->interface = interface;

    system_init_group(&group->base, interface->driver->module.name);
    system_group_add(&root, &group->base.node);
    system_init_stream(&group->data, "data", data_read, data_write);
    system_group_add(&group->base, &group->data.node);

    ninterfaces++;

}

void kbd_register_protocol(struct kbd_protocol *protocol)
{

}

void kbd_unregister_interface(struct kbd_interface *interface)
{

}

void kbd_unregister_protocol(struct kbd_protocol *protocol)
{

}

void kbd_init_interface(struct kbd_interface *interface, struct base_driver *driver, unsigned int (*read_data)(struct kbd_interface *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_data)(struct kbd_interface *self, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct kbd_interface));

    interface->driver = driver;
    interface->read_data = read_data;
    interface->write_data = write_data;

}

void kbd_init_protocol(struct kbd_protocol *protocol, char *name)
{

    memory_clear(protocol, sizeof (struct kbd_protocol));

    protocol->name = name;

}

void init()
{

    ninterfaces = 0;

    system_init_group(&root, "kbd");
    system_register_node(&root.node);

}

void destroy()
{

    system_unregister_node(&root.node);

}

