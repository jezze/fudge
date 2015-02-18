#include <fudge.h>
#include <kernel.h>
#include <base/base.h>
#include <system/system.h>
#include "block.h"

static struct system_node root;

static unsigned int interfacenode_dataread(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct block_interface *interface = (struct block_interface *)self->parent;

    return interface->rdata(offset, count, buffer);

}

void block_registerinterface(struct block_interface *interface, unsigned int id)
{

    system_registerinterface(&interface->base, id);
    system_addchild(&interface->base.root, &interface->data);
    system_addchild(&root, &interface->base.root);

}

void block_unregisterinterface(struct block_interface *interface)
{

    system_unregisterinterface(&interface->base);
    system_removechild(&interface->base.root, &interface->data);
    system_removechild(&root, &interface->base.root);

}

void block_initinterface(struct block_interface *interface, struct base_driver *driver, unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*wdata)(unsigned int offset, unsigned int count, void *buffer))
{

    system_initinterface(&interface->base, driver->name);
    system_initnode(&interface->data, SYSTEM_NODETYPE_NORMAL, "data");

    interface->rdata = rdata;
    interface->wdata = wdata;
    interface->data.read = interfacenode_dataread;

}

void module_init()
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "block");

}

void module_register()
{

    system_registernode(&root);

}

void module_unregister()
{

    system_unregisternode(&root);

}

