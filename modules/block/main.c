#include <fudge/module.h>
#include <base/base.h>
#include <system/system.h>
#include "block.h"

struct block_interface_group
{

    struct system_group base;
    struct block_interface *interface;

};

static struct system_group root;
static struct block_interface_group interfaces[32];
static unsigned int ninterfaces;

void block_register_interface(struct block_interface *interface)
{

    struct block_interface_group *group = &interfaces[ninterfaces];

    group->interface = interface;

    system_init_group(&group->base, interface->driver->module.name);
    system_group_add(&root, &group->base.node);

    ninterfaces++;

}

void block_register_protocol(struct block_protocol *protocol)
{

}

void block_unregister_interface(struct block_interface *interface)
{

}

void block_unregister_protocol(struct block_protocol *protocol)
{

}

void block_init_interface(struct block_interface *interface, struct base_driver *driver, unsigned int (*read)(struct block_interface *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct block_interface *self, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct block_interface));

    interface->driver = driver;
    interface->read = read;
    interface->write = write;


}

void block_init_protocol(struct block_protocol *protocol, char *name)
{

    memory_clear(protocol, sizeof (struct block_protocol));

    protocol->name = name;

}

void init()
{

    ninterfaces = 0;

    system_init_group(&root, "block");
    system_register_node(&root.node);

}

void destroy()
{

    system_unregister_node(&root.node);

}

