#include <fudge.h>
#include <kernel.h>
#include "block.h"

static struct node_operands operands;

static unsigned int onreadblockrequest(struct block_interface *interface, unsigned int source, unsigned int count, void *data)
{

    struct event_blockrequest *blockrequest = data;

    return interface->onreadblockrequest(source, blockrequest->count, blockrequest->offset);

}

static unsigned int operands_place(struct resource *resource, unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    struct block_interface *interface = resource->data;

    switch (event)
    {

    case EVENT_LINK:
        return kernel_linknode(target, source);

    case EVENT_UNLINK:
        return kernel_unlinknode(target, source);

    case EVENT_BLOCKREADREQUEST:
        return onreadblockrequest(interface, source, count, data);

    }

    return MESSAGE_UNIMPLEMENTED;

}

void block_notifyblockresponse(struct block_interface *interface, void *buffer, unsigned int count)
{

    kernel_notify(interface->inode, EVENT_BLOCKREADRESPONSE, count, buffer);

}

void block_registerinterface(struct block_interface *interface)
{

    resource_register(&interface->resource);

}

void block_unregisterinterface(struct block_interface *interface)
{

    resource_unregister(&interface->resource);

}

void block_initinterface(struct block_interface *interface, unsigned int id, unsigned int (*onreadblockrequest)(unsigned int source, unsigned int count, unsigned int offset))
{

    resource_init(&interface->resource, RESOURCE_BLOCKINTERFACE, interface);

    interface->id = id;
    interface->inode = pool_picknode();
    interface->onreadblockrequest = onreadblockrequest;

    if (interface->inode)
    {

        struct node *node = pool_getnode(interface->inode);

        node_reset(node, "block", &interface->resource, &operands);

    }

}

void module_init(void)
{

    node_operands_init(&operands, 0, operands_place);

}

