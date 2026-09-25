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

        return kernel_place(interface->service.inode, source, EVENT_BLOCKINFO, sizeof (struct event_blockinfo), &blockinfo);

    }

    return MESSAGE_FAILED;

}

static unsigned int onblockreadrequest(struct block_interface *interface, unsigned int source, unsigned int count, void *data)
{

    if (interface->startsession)
    {

        struct event_blockrequest *request = data;
        struct block_session *session = &interface->sessions[0];

        if (!session->source)
        {

            session->type = BLOCK_TYPE_READ;
            session->source = source;
            session->start = request->offset;
            session->count = request->count;
            session->offset = 0;

            interface->startsession(session);

            return MESSAGE_OK;

        }

        return MESSAGE_RETRY;

    }

    return MESSAGE_FAILED;

}

static unsigned int onblockwriterequest(struct block_interface *interface, unsigned int source, unsigned int count, void *data)
{

    if (interface->startsession)
    {

        struct event_blockrequest *request = data;
        struct block_session *session = &interface->sessions[0];

        if (!session->source)
        {

            session->type = BLOCK_TYPE_WRITE;
            session->source = source;
            session->start = request->offset;
            session->count = request->count;
            session->offset = 0;

            interface->startsession(session);

            return MESSAGE_OK;

        }

        return MESSAGE_RETRY;

    }

    return MESSAGE_FAILED;

}

static unsigned int operands_place(unsigned int target, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    struct node *tnode = pool_getnode(target);

    if (tnode)
    {

        struct block_interface *interface = tnode->resource->data;

        switch (event)
        {

        case EVENT_LINK:
            return kernel_linknode(&interface->service.links, source);

        case EVENT_UNLINK:
            return kernel_unlinknode(&interface->service.links, source);

        case EVENT_INFO:
            return oninfo(interface, source);

        case EVENT_BLOCKREADREQUEST:
            return onblockreadrequest(interface, source, count, data);

        case EVENT_BLOCKWRITEREQUEST:
            return onblockwriterequest(interface, source, count, data);

        }

    }

    return MESSAGE_FAILED;

}

struct block_session *block_getsession(struct block_interface *interface)
{

    return &interface->sessions[0];

}

void block_session_done(struct block_interface *interface, struct block_session *session)
{

    struct event_blockresponse response;

    response.count = session->count;

    switch (session->type)
    {

    case BLOCK_TYPE_READ:
        kernel_place(interface->service.inode, session->source, EVENT_BLOCKREADRESPONSE, sizeof (struct event_blockresponse), &response);

        break;

    case BLOCK_TYPE_WRITE:
        kernel_place(interface->service.inode, session->source, EVENT_BLOCKWRITERESPONSE, sizeof (struct event_blockresponse), &response);

        break;

    }

    session->source = 0;

}

void block_registerinterface(struct block_interface *interface)
{

    unsigned int inode = pool_picknode();

    if (inode)
    {

        struct node *node = pool_getnode(inode);

        node_reset(node, &interface->resource, &operands);

    }

    resource_register(&interface->resource);
    service_register(&interface->service, inode);

}

void block_unregisterinterface(struct block_interface *interface)
{

    resource_unregister(&interface->resource);
    service_unregister(&interface->service);

}

void block_initinterface(struct block_interface *interface, unsigned int id, void (*oninfo)(struct event_blockinfo *blockinfo), void (*startsession)(struct block_session *session))
{

    resource_init(&interface->resource, RESOURCE_BLOCKINTERFACE, interface);
    service_init(&interface->service, "block");

    interface->id = id;
    interface->oninfo = oninfo;
    interface->startsession = startsession;

}

void module_init(void)
{

    node_operands_init(&operands, 0, operands_place);

}

