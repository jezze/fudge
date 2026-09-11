#include <fudge.h>
#include <kernel.h>
#include "block.h"

static struct node_operands operands;

static unsigned int oninfo(struct block_interface *interface, unsigned int source)
{

    return interface->oninfo(source);

}

static unsigned int onblockreadrequest(struct block_interface *interface, unsigned int source, unsigned int count, void *data)
{

    struct event_blockrequest *request = data;

    return interface->onblockreadrequest(source, request->count, request->offset);

}

static unsigned int onblockwriterequest(struct block_interface *interface, unsigned int source, unsigned int count, void *data)
{

    struct event_blockrequest *request = data;

    return interface->onblockwriterequest(source, request->count, request->offset);

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

    case EVENT_INFO:
        return oninfo(interface, source);

    case EVENT_BLOCKREADREQUEST:
        return onblockreadrequest(interface, source, count, data);

    case EVENT_BLOCKWRITEREQUEST:
        return onblockwriterequest(interface, source, count, data);

    }

    return MESSAGE_FAILED;

}

void block_registerinterface(struct block_interface *interface)
{

    resource_register(&interface->resource);

}

void block_unregisterinterface(struct block_interface *interface)
{

    resource_unregister(&interface->resource);

}

void block_initinterface(struct block_interface *interface, unsigned int id, unsigned int (*oninfo)(unsigned int source), unsigned int (*onblockreadrequest)(unsigned int source, unsigned int count, unsigned int offset), unsigned int (*onblockwriterequest)(unsigned int source, unsigned int count, unsigned int offset))
{

    resource_init(&interface->resource, RESOURCE_BLOCKINTERFACE, interface);

    interface->id = id;
    interface->inode = pool_picknode();
    interface->oninfo = oninfo;
    interface->onblockreadrequest = onblockreadrequest;
    interface->onblockwriterequest = onblockwriterequest;

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

