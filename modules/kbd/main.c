#include <fudge/memory.h>
#include <system/system.h>
#include <base/base.h>
#include "kbd.h"

static struct system_group root;

void kbd_register_interface(struct kbd_interface *interface)
{

    system_group_add(&root, &interface->node.root.node);

}

void kbd_register_protocol(struct kbd_protocol *protocol)
{

}

void kbd_unregister_interface(struct kbd_interface *interface)
{

    system_group_remove(&root, &interface->node.root.node);

}

void kbd_unregister_protocol(struct kbd_protocol *protocol)
{

}

void kbd_init_interface(struct kbd_interface *interface, struct base_driver *driver, unsigned int (*read)(struct kbd_interface *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct kbd_interface *self, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct kbd_interface));

    interface->driver = driver;
    interface->read = read;
    interface->write = write;

    system_init_group(&interface->node.root, interface->driver->module.name);

}

void kbd_init_protocol(struct kbd_protocol *protocol, char *name)
{

    memory_clear(protocol, sizeof (struct kbd_protocol));

    protocol->name = name;

}

void init()
{

    system_init_group(&root, "kbd");
    system_register_node(&root.node);

}

void destroy()
{

    system_unregister_node(&root.node);

}

