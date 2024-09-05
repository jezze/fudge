#include <fudge.h>
#include <kernel.h>
#include "block.h"

void block_notifyblockresponse(struct block_interface *interface, void *buffer, unsigned int count)
{

    kernel_notify(interface->ichannel, EVENT_BLOCKRESPONSE, count, buffer);

}

void block_registerinterface(struct block_interface *interface)
{

    resource_register(&interface->resource);

}

void block_unregisterinterface(struct block_interface *interface)
{

    resource_unregister(&interface->resource);

}

void block_initinterface(struct block_interface *interface, unsigned int id, unsigned int ichannel)
{

    resource_init(&interface->resource, RESOURCE_BLOCKINTERFACE, interface);

    interface->id = id;
    interface->ichannel = ichannel;

}

