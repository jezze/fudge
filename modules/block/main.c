#include <fudge/module.h>
#include <system/system.h>
#include <base/base.h>
#include "block.h"

static struct system_group root;

void block_register_interface(struct block_interface *interface)
{

    system_group_add(&root, &interface->node.root.node);

}

void block_register_protocol(struct block_protocol *protocol)
{

    system_group_add(&root, &protocol->node.root.node);

}

void block_unregister_interface(struct block_interface *interface)
{

    system_group_remove(&root, &interface->node.root.node);

}

void block_unregister_protocol(struct block_protocol *protocol)
{

}

void block_init_interface(struct block_interface *interface, struct base_driver *driver, unsigned int (*read)(struct block_interface *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct block_interface *self, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct block_interface));
    system_init_group(&interface->node.root, driver->module.name);

    interface->driver = driver;
    interface->read = read;
    interface->write = write;


}

void block_init_protocol(struct block_protocol *protocol, char *name)
{

    memory_clear(protocol, sizeof (struct block_protocol));
    system_init_group(&protocol->node.root, name);

    protocol->name = name;

}

void init()
{

    system_init_group(&root, "block");
    system_register_node(&root.node);

}

void destroy()
{

    system_unregister_node(&root.node);

}

