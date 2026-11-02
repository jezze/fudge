#include <fudge.h>
#include <kernel.h>
#include "block.h"

static struct node_operands operands;
static struct service service;

static unsigned int oninfo(struct block_interface *interface, unsigned int source)
{

    if (interface->blocksize)
    {

        struct event_blockinfo blockinfo;

        blockinfo.buffer = KERNEL_VBUFFER;
        blockinfo.buffersize = TASK_BUFFERSIZE;
        blockinfo.blocksize = interface->blocksize;
        blockinfo.blockcount = interface->blockcount;

        return kernel_place(interface->resource.inode, source, EVENT_BLOCKINFO, sizeof (struct event_blockinfo), &blockinfo);

    }

    return MESSAGE_FAILED;

}

static unsigned int onblockreadrequest(struct block_interface *interface, unsigned int source, unsigned int count, void *data)
{

    if (count < sizeof (struct event_blockrequest))
        return MESSAGE_FAILED;

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

    if (count < sizeof (struct event_blockrequest))
        return MESSAGE_FAILED;

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

    struct resource *resource = pool_getnoderesource(target);

    if (resource)
    {

        struct block_interface *interface = resource->data;

        switch (event)
        {

        case EVENT_LINK:
            return kernel_linknode(&interface->resource, source);

        case EVENT_UNLINK:
            return kernel_unlinknode(&interface->resource, source);

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
        kernel_place(interface->resource.inode, session->source, EVENT_BLOCKREADRESPONSE, sizeof (struct event_blockresponse), &response);

        break;

    case BLOCK_TYPE_WRITE:
        kernel_place(interface->resource.inode, session->source, EVENT_BLOCKWRITERESPONSE, sizeof (struct event_blockresponse), &response);

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

void block_initinterface(struct block_interface *interface, unsigned int blocksize, void (*startsession)(struct block_session *session))
{

    resource_init(&interface->resource, RESOURCE_BLOCKINTERFACE, interface);

    interface->blocksize = blocksize;
    interface->blockcount = 0;
    interface->startsession = startsession;

}

void module_init(void)
{

    node_operands_init(&operands, 0, operands_place, 0);

    service_init(&service);

}

void module_register(void)
{

    service_registerdirectory(&service, "block", RESOURCE_BLOCKINTERFACE, &operands);

}

void module_unregister(void)
{

    service_unregister(&service);

}

