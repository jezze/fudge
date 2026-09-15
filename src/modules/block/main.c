#include <fudge.h>
#include <kernel.h>
#include "block.h"

static struct node_operands operands;

static unsigned int oninfo(struct block_interface *interface, unsigned int source)
{

    if (interface->oninfo)
    {

        struct event_blockinfo blockinfo;

        interface->oninfo(&blockinfo);

        return kernel_place(interface->inode, source, EVENT_BLOCKINFO, sizeof (struct event_blockinfo), &blockinfo);

    }

    return MESSAGE_FAILED;

}

static unsigned int onblockreadrequest(struct block_interface *interface, unsigned int source, unsigned int count, void *data)
{

    if (interface->onblockreadrequest)
    {

        struct event_blockrequest *request = data;
        struct block_session *session = &interface->sessions[0];

        if (!session->source)
        {

            session->type = 1;
            session->source = source;
            session->start = request->offset;
            session->count = request->count;
            session->offset = 0;

            interface->onblockreadrequest(session);

            return MESSAGE_OK;

        }

        return MESSAGE_RETRY;

    }

    return MESSAGE_FAILED;

}

static unsigned int onblockwriterequest(struct block_interface *interface, unsigned int source, unsigned int count, void *data)
{

    if (interface->onblockwriterequest)
    {

        struct event_blockrequest *request = data;
        struct block_session *session = &interface->sessions[0];

        if (!session->source)
        {

            session->type = 2;
            session->source = source;
            session->start = request->offset;
            session->count = request->count;
            session->offset = 0;

            interface->onblockwriterequest(session);

            return MESSAGE_OK;

        }

        return MESSAGE_RETRY;

    }

    return MESSAGE_FAILED;

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

void block_session_done(struct block_interface *interface, struct block_session *session)
{

    struct event_blockresponse response;

    response.count = session->count;

    switch (session->type)
    {

    case 1:
        kernel_place(interface->inode, session->source, EVENT_BLOCKREADRESPONSE, sizeof (struct event_blockresponse), &response);

        break;

    case 2:
        kernel_place(interface->inode, session->source, EVENT_BLOCKWRITERESPONSE, sizeof (struct event_blockresponse), &response);

        break;

    }

    session->source = 0;

}

void block_registerinterface(struct block_interface *interface)
{

    resource_register(&interface->resource);

}

void block_unregisterinterface(struct block_interface *interface)
{

    resource_unregister(&interface->resource);

}

void block_initinterface(struct block_interface *interface, unsigned int id, void (*oninfo)(struct event_blockinfo *blockinfo), void (*onblockreadrequest)(struct block_session *session), void (*onblockwriterequest)(struct block_session *session))
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

