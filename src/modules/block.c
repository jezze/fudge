#include <fudge.h>
#include <kernel.h>
#include "block.h"

static struct unitsrv unitsrv;

static unsigned int oninfo(struct block_interface *interface, unsigned int source)
{

    if (interface->blocksize)
    {

        struct message_blockinfo blockinfo;

        blockinfo.blocksize = interface->blocksize;
        blockinfo.blockcount = interface->blockcount;

        return kernel_placereply(source, interface->resource.inode, MESSAGE_TYPE_BLOCKINFO, sizeof (struct message_blockinfo), &blockinfo);

    }

    return MESSAGE_FAILED;

}

static unsigned int onblockreadrequest(struct block_interface *interface, unsigned int source, unsigned int count, void *data)
{

    struct message_blockrequest *request = message_validate(MESSAGE_TYPE_BLOCKREADREQUEST, data, count);

    if (interface->startsession && request)
    {

        struct block_session *session = &interface->sessions[0];
        struct page_lease lease = page_borrow(source, request->page);

        if (lease.status != MESSAGE_OK)
            return MESSAGE_FAILED;

        if (!session->source)
        {

            unsigned int status = kernel_keepreply(source);

            if (status != MESSAGE_OK)
                return status;

            /* the driver fills the page the requester brought, which stays the requester's */
            session->type = BLOCK_TYPE_READ;
            session->source = source;
            session->buffer = (unsigned long)lease.data;
            session->sector = request->sector;
            session->count = (request->count < PAGE_SIZE) ? request->count : PAGE_SIZE;
            session->offset = 0;

            status = interface->startsession(session);

            if (status != MESSAGE_OK && session->source)
            {

                session->source = 0;

                kernel_unreserve(source);

            }

            return status;

        }

        return MESSAGE_RETRY;

    }

    return MESSAGE_FAILED;

}

static unsigned int onblockwriterequest(struct block_interface *interface, unsigned int source, unsigned int count, void *data)
{

    struct message_blockrequest *request = message_validate(MESSAGE_TYPE_BLOCKWRITEREQUEST, data, count);

    if (interface->startsession && request)
    {

        struct block_session *session = &interface->sessions[0];
        struct page_lease lease = page_borrow(source, request->page);

        if (lease.status != MESSAGE_OK)
            return MESSAGE_FAILED;

        if (!session->source)
        {

            unsigned int status = kernel_keepreply(source);

            if (status != MESSAGE_OK)
                return status;

            session->type = BLOCK_TYPE_WRITE;
            session->source = source;
            session->buffer = (unsigned long)lease.data;
            session->sector = request->sector;
            session->count = (request->count < PAGE_SIZE) ? request->count : PAGE_SIZE;
            session->offset = 0;

            status = interface->startsession(session);

            if (status != MESSAGE_OK && session->source)
            {

                session->source = 0;

                kernel_unreserve(source);

            }

            return status;

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

        case MESSAGE_TYPE_INFO:
            return oninfo(interface, source);

        case MESSAGE_TYPE_BLOCKREADREQUEST:
            return onblockreadrequest(interface, source, count, data);

        case MESSAGE_TYPE_BLOCKWRITEREQUEST:
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

    struct message_blockresponse response;

    response.count = session->count;

    switch (session->type)
    {

    case BLOCK_TYPE_READ:
        kernel_placereserved(session->source, interface->resource.inode, MESSAGE_TYPE_BLOCKREADRESPONSE, sizeof (struct message_blockresponse), &response);

        break;

    case BLOCK_TYPE_WRITE:
        kernel_placereserved(session->source, interface->resource.inode, MESSAGE_TYPE_BLOCKWRITERESPONSE, sizeof (struct message_blockresponse), &response);

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

void block_initinterface(struct block_interface *interface, unsigned int blocksize, unsigned int (*startsession)(struct block_session *session))
{

    resource_init(&interface->resource, RESOURCE_BLOCKINTERFACE, interface, operands_place);

    interface->blocksize = blocksize;
    interface->blockcount = 0;
    interface->startsession = startsession;

}

void module_init(void)
{

    unitsrv_init(&unitsrv, RESOURCE_BLOCKINTERFACE, "block");

}

void module_register(void)
{

    unitsrv_register(&unitsrv);

}

void module_unregister(void)
{

    unitsrv_unregister(&unitsrv);

}

