#include <fudge.h>
#include "resource.h"
#include "pool.h"
#include "service.h"
#include "kernel.h"
#include "namesrv.h"

#define LISTRECORDS                     8

static struct resource server;
static struct service service;

static unsigned int findservice(char *name, unsigned int length)
{

    struct resource *resource = 0;
    unsigned int inode = 0;

    resource_lock();

    while ((resource = resource_foreachtype_unsafe(resource, RESOURCE_SERVICE)))
    {

        struct service *current = resource->data;

        if (length < SERVICE_NAMESIZE && !current->name[length] && buffer_match(current->name, name, length))
        {

            inode = current->inode;

            break;

        }

    }

    resource_unlock();

    return inode;

}

static unsigned int walk(unsigned int parent, unsigned int length, char *path)
{

    unsigned int id = (parent) ? parent : MESSAGE_ROOT;
    unsigned int offset = 0;

    while (offset < length)
    {

        char *cp = path + offset;
        unsigned int cl = buffer_findbyte(cp, length - offset, '/');

        if (cl == 2 && buffer_match(cp, "..", 2))
            id = MESSAGE_ROOT;
        else if (cl && (cl != 1 || !buffer_match(cp, ".", 1)))
            id = (id == MESSAGE_ROOT) ? findservice(cp, cl) : 0;

        if (!id)
            return 0;

        offset += cl + 1;

    }

    return id;

}

static unsigned int onreadrequest(unsigned int source, unsigned int count, void *data)
{

    struct message_readrequest *request = message_validate(MESSAGE_TYPE_READREQUEST, data, count);
    unsigned char buffer[sizeof (struct message_readresponse) + sizeof (struct record) * LISTRECORDS];
    struct message_readresponse *response = (struct message_readresponse *)buffer;
    struct record *records = (struct record *)(response + 1);

    if (request)
    {

        struct resource *resource = 0;
        unsigned int c = 0;
        unsigned int i;

        for (i = request->offset; request->id == MESSAGE_ROOT && i < 2 && (c + 1) * sizeof (struct record) <= request->count; i++)
            record_init(&records[c++], MESSAGE_ROOT, RECORD_TYPE_DIRECTORY, 0, i + 1, i + 1, "..");

        resource_lock();

        for (i = 2; request->id == MESSAGE_ROOT && c < LISTRECORDS && (c + 1) * sizeof (struct record) <= request->count && (resource = resource_foreachtype_unsafe(resource, RESOURCE_SERVICE)); i++)
        {

            struct service *current = resource->data;

            if (i >= request->offset)
                record_init(&records[c++], current->inode, RECORD_TYPE_DIRECTORY, 0, i + 1, cstring_length(current->name), current->name);

        }

        resource_unlock();

        response->count = c * sizeof (struct record);
        response->buffer = 0;

        return kernel_place(MESSAGE_ROOT, source, MESSAGE_TYPE_READRESPONSE, sizeof (struct message_readresponse) + response->count, buffer);

    }

    return MESSAGE_FAILED;

}

static unsigned int onstatrequest(unsigned int source, unsigned int count, void *data)
{

    struct message_statrequest *request = message_validate(MESSAGE_TYPE_STATREQUEST, data, count);
    unsigned char buffer[sizeof (struct message_statresponse) + sizeof (struct record)];
    struct message_statresponse *response = (struct message_statresponse *)buffer;
    struct record *record = (struct record *)(response + 1);

    if (request)
    {

        record_init(record, MESSAGE_ROOT, RECORD_TYPE_DIRECTORY, 0, 1, 1, ".");

        response->count = (request->id == MESSAGE_ROOT) ? sizeof (struct record) : 0;

        return kernel_place(MESSAGE_ROOT, source, MESSAGE_TYPE_STATRESPONSE, sizeof (struct message_statresponse) + response->count, buffer);

    }

    return MESSAGE_FAILED;

}

static unsigned int onwalkrequest(unsigned int source, unsigned int count, void *data)
{

    struct message_walkrequest *request = message_validate(MESSAGE_TYPE_WALKREQUEST, data, count);
    struct message_walkresponse response;

    if (request)
    {

        response.id = walk(request->parent, request->length, (char *)(request + 1));

        return kernel_place(MESSAGE_ROOT, source, MESSAGE_TYPE_WALKRESPONSE, sizeof (struct message_walkresponse), &response);

    }

    return MESSAGE_FAILED;

}

static unsigned int operands_place(unsigned int target, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    switch (event)
    {

    case MESSAGE_TYPE_READREQUEST:
        return onreadrequest(source, count, data);

    case MESSAGE_TYPE_STATREQUEST:
        return onstatrequest(source, count, data);

    case MESSAGE_TYPE_WALKREQUEST:
        return onwalkrequest(source, count, data);

    }

    return MESSAGE_FAILED;

}

void namesrv_setup(void)
{

    resource_init(&server, 0, 0, operands_place);
    service_init(&service, "namesrv");
    service_register(&service, MESSAGE_ROOT);
    pool_setroot(&server);

}

