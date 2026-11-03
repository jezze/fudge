#include <fudge.h>
#include <hash.h>
#include "resource.h"
#include "pool.h"
#include "server.h"
#include "service.h"
#include "kernel.h"
#include "namesrv.h"

#define LISTRECORDS                     8

static struct server server;
static struct service service;

static unsigned int findservice(char *name, unsigned int length)
{

    unsigned int namehash = djb_hash(length, name);
    struct resource *resource = 0;
    unsigned int inode = 0;

    resource_lock();

    while ((resource = resource_foreachtype_unsafe(resource, RESOURCE_SERVICE)))
    {

        struct service *current = resource->data;

        if (current->namehash == namehash && length < SERVICE_NAMESIZE && !current->name[length] && buffer_match(current->name, name, length))
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

    struct event_readrequest *request = data;
    struct resource *resource = 0;
    unsigned char buffer[sizeof (struct event_readresponse) + sizeof (struct record) * LISTRECORDS];
    struct event_readresponse *response = (struct event_readresponse *)buffer;
    struct record *records = (struct record *)(response + 1);
    unsigned int c = 0;
    unsigned int i;

    if (count < sizeof (struct event_readrequest))
        return MESSAGE_FAILED;

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

    return kernel_place(MESSAGE_ROOT, source, EVENT_READRESPONSE, sizeof (struct event_readresponse) + response->count, buffer);

}

static unsigned int onstatrequest(unsigned int source, unsigned int count, void *data)
{

    struct event_statrequest *request = data;
    unsigned char buffer[sizeof (struct event_statresponse) + sizeof (struct record)];
    struct event_statresponse *response = (struct event_statresponse *)buffer;
    struct record *record = (struct record *)(response + 1);

    if (count < sizeof (struct event_statrequest))
        return MESSAGE_FAILED;

    record_init(record, MESSAGE_ROOT, RECORD_TYPE_DIRECTORY, 0, 1, 1, ".");

    response->count = (request->id == MESSAGE_ROOT) ? sizeof (struct record) : 0;

    return kernel_place(MESSAGE_ROOT, source, EVENT_STATRESPONSE, sizeof (struct event_statresponse) + response->count, buffer);

}

static unsigned int onwalkrequest(unsigned int source, unsigned int count, void *data)
{

    struct event_walkrequest *request = data;
    struct event_walkresponse response;

    if (count < sizeof (struct event_walkrequest) || count - sizeof (struct event_walkrequest) < request->length)
        return MESSAGE_FAILED;

    response.id = walk(request->parent, request->length, (char *)(request + 1));

    return kernel_place(MESSAGE_ROOT, source, EVENT_WALKRESPONSE, sizeof (struct event_walkresponse), &response);

}

static unsigned int operands_place(unsigned int target, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    switch (event)
    {

    case EVENT_READREQUEST:
        return onreadrequest(source, count, data);

    case EVENT_STATREQUEST:
        return onstatrequest(source, count, data);

    case EVENT_WALKREQUEST:
        return onwalkrequest(source, count, data);

    }

    return MESSAGE_FAILED;

}

void namesrv_setup(void)
{

    server_init(&server, 0, operands_place, 0);
    service_init(&service, "namesrv");
    service_register(&service, MESSAGE_ROOT);
    pool_setroot(&server.resource);

}

