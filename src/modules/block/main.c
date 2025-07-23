#include <fudge.h>
#include <kernel.h>
#include "block.h"

static struct service service;

static struct resource *service_foreach(struct resource *current)
{

    return resource_foreachtype(current, RESOURCE_BLOCKINTERFACE);

}

static unsigned int service_getinode(struct resource *current, unsigned int index)
{

    struct block_interface *interface = current->data;

    return interface->inode;

}

static unsigned int onblockrequest(struct block_interface *interface, unsigned int source, unsigned int count, void *data)
{

    struct event_blockrequest *blockrequest = data;

    return interface->onblockrequest(source, blockrequest->count, blockrequest->sector);

}

static unsigned int service_place(unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    struct block_interface *interface = kernel_getnodeinterface(target);

    switch (event)
    {

    case EVENT_LINK:
        return kernel_linknode(target, source);

    case EVENT_UNLINK:
        return kernel_unlinknode(target, source);

    case EVENT_BLOCKREQUEST:
        return onblockrequest(interface, source, count, data);

    }

    return MESSAGE_UNIMPLEMENTED;

}

void block_notifyblockresponse(struct block_interface *interface, void *buffer, unsigned int count)
{

    kernel_notify(interface->inode, EVENT_BLOCKRESPONSE, count, buffer);

}

void block_registerinterface(struct block_interface *interface)
{

    resource_register(&interface->resource);

}

void block_unregisterinterface(struct block_interface *interface)
{

    resource_unregister(&interface->resource);

}

void block_initinterface(struct block_interface *interface, unsigned int id, unsigned int (*onblockrequest)(unsigned int source, unsigned int count, unsigned int sector))
{

    resource_init(&interface->resource, RESOURCE_BLOCKINTERFACE, interface);

    interface->id = id;
    interface->inode = kernel_addnode("block", &interface->resource, &service);
    interface->onblockrequest = onblockrequest;

}

void module_init(void)
{

    service_init(&service, "block", service_foreach, service_getinode, 0, service_place);
    service_register(&service);

}

