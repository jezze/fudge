#include <fudge.h>
#include <kernel.h>
#include <system/system.h>
#include "block.h"

static unsigned int wait;

void block_notify(struct block_interface *interface, unsigned int count, void *buffer)
{

    scheduler_mailboxes_send(&interface->data.mailboxes, count, buffer);

    wait = 0;

}

static unsigned int interfacenode_dataread(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct block_interface *interface = self->resource->data;

    if (offset > 512)
        return 0;

    if (wait == 0)
    {

        interface->rdata(offset, count, buffer);
        wait = 1;

    }

    return scheduler_mailboxes_readactive(&self->mailboxes, count, buffer);

}

void block_registerinterface(struct block_interface *interface, unsigned int id)
{

    system_addchild(&interface->root, &interface->data);
    system_registernode(&interface->root);

    interface->id = id;

}

void block_unregisterinterface(struct block_interface *interface)
{

    system_removechild(&interface->root, &interface->data);
    system_unregisternode(&interface->root);

}

void block_initinterface(struct block_interface *interface, unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*wdata)(unsigned int offset, unsigned int count, void *buffer))
{

    resource_init(&interface->resource, 0, interface);
    system_initnode(&interface->root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "block");
    system_initnode(&interface->data, SYSTEM_NODETYPE_MAILBOX, "data");

    interface->rdata = rdata;
    interface->wdata = wdata;
    interface->data.resource = &interface->resource;
    interface->data.read = interfacenode_dataread;

}

void module_init()
{

}

void module_register()
{

}

void module_unregister()
{

}

