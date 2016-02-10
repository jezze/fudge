#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "block.h"

void block_notify(struct block_interface *interface, unsigned int count, void *buffer)
{

}

static unsigned int interfacedata_read(struct system_node *self, struct list_item *link, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

void block_registerinterface(struct block_interface *interface, unsigned int id)
{

    resource_register(&interface->resource);
    system_addchild(&interface->root, &interface->data);
    system_registernode(&interface->root);

    interface->id = id;

}

void block_unregisterinterface(struct block_interface *interface)
{

    resource_unregister(&interface->resource);
    system_removechild(&interface->root, &interface->data);
    system_unregisternode(&interface->root);

}

void block_initinterface(struct block_interface *interface, unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*wdata)(unsigned int offset, unsigned int count, void *buffer))
{

    resource_init(&interface->resource, RESOURCE_BLOCKINTERFACE, interface);
    system_initnode(&interface->root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "block");
    system_initnode(&interface->data, SYSTEM_NODETYPE_MAILBOX, "data");

    interface->rdata = rdata;
    interface->wdata = wdata;
    interface->data.resource = &interface->resource;
    interface->data.read = interfacedata_read;

}

