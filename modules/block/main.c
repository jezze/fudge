#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include <base/base.h>
#include "block.h"

static struct system_node root;

static unsigned int interfacenode_dataread(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct block_interfacenode *node = (struct block_interfacenode *)self->parent;

    return node->interface->rdata(offset, count, buffer);

}

void block_registerinterface(struct block_interface *interface, struct base_bus *bus, unsigned int id)
{

    base_registerinterface(&interface->base, bus, id);
    system_addchild(&root, &interface->node.base);
    system_addchild(&interface->node.base, &interface->node.data);

}

void block_unregisterinterface(struct block_interface *interface)
{

    base_unregisterinterface(&interface->base);
    system_removechild(&interface->node.base, &interface->node.data);
    system_removechild(&root, &interface->node.base);

}

void block_initinterface(struct block_interface *interface, struct base_driver *driver, unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*wdata)(unsigned int offset, unsigned int count, void *buffer))
{

    base_initinterface(&interface->base, driver);
    system_initnode(&interface->node.base, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, driver->name);
    system_initnode(&interface->node.data, SYSTEM_NODETYPE_NORMAL, "data");

    interface->rdata = rdata;
    interface->wdata = wdata;
    interface->node.interface = interface;
    interface->node.data.read = interfacenode_dataread;

}

void init()
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "block");
    system_registernode(&root);

}

void destroy()
{

    system_unregisternode(&root);

}

