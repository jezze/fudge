#include <fudge.h>
#include <hash.h>
#include "resource.h"
#include "pool.h"
#include "server.h"
#include "service.h"
#include "kernel.h"
#include "namesrv.h"

#define LISTRECORDS                     8

struct listing
{

    struct event_readresponse response;
    struct record records[LISTRECORDS];

};

struct statresult
{

    struct event_statresponse response;
    struct record record;

};

static struct server rootserver;
static struct service rootservice;

static unsigned int findservice(char *name, unsigned int length)
{

    unsigned int namehash = djb_hash(length, name);
    struct resource *resource = 0;
    unsigned int inode = 0;

    resource_lock();

    while ((resource = resource_foreachtype_unsafe(resource, RESOURCE_SERVICE)))
    {

        struct service *service = resource->data;

        if (service->namehash == namehash && length < SERVICE_NAMESIZE && !service->name[length] && buffer_match(service->name, name, length))
        {

            inode = service->inode;

            break;

        }

    }

    resource_unlock();

    return inode;

}

static unsigned int walkroot(unsigned int parent, unsigned int length, char *path)
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

static unsigned int onrootreadrequest(unsigned int source, unsigned int count, void *data)
{

    struct event_readrequest *request = data;
    struct resource *resource = 0;
    struct listing listing;
    unsigned int c = 0;
    unsigned int i;

    if (count < sizeof (struct event_readrequest))
        return MESSAGE_FAILED;

    for (i = request->offset; request->id == MESSAGE_ROOT && i < 2 && (c + 1) * sizeof (struct record) <= request->count; i++)
        record_init(&listing.records[c++], MESSAGE_ROOT, RECORD_TYPE_DIRECTORY, 0, i + 1, i + 1, "..");

    resource_lock();

    for (i = 2; request->id == MESSAGE_ROOT && c < LISTRECORDS && (c + 1) * sizeof (struct record) <= request->count && (resource = resource_foreachtype_unsafe(resource, RESOURCE_SERVICE)); i++)
    {

        struct service *service = resource->data;

        if (i >= request->offset)
            record_init(&listing.records[c++], service->inode, RECORD_TYPE_DIRECTORY, 0, i + 1, cstring_length(service->name), service->name);

    }

    resource_unlock();

    listing.response.count = c * sizeof (struct record);
    listing.response.buffer = 0;

    return kernel_place(MESSAGE_ROOT, source, EVENT_READRESPONSE, sizeof (struct event_readresponse) + listing.response.count, &listing);

}

static unsigned int onrootstatrequest(unsigned int source, unsigned int count, void *data)
{

    struct event_statrequest *request = data;
    struct statresult result;

    if (count < sizeof (struct event_statrequest))
        return MESSAGE_FAILED;

    record_init(&result.record, MESSAGE_ROOT, RECORD_TYPE_DIRECTORY, 0, 1, 1, ".");

    result.response.count = (request->id == MESSAGE_ROOT) ? sizeof (struct record) : 0;

    return kernel_place(MESSAGE_ROOT, source, EVENT_STATRESPONSE, sizeof (struct event_statresponse) + result.response.count, &result);

}

static unsigned int onrootwalkrequest(unsigned int source, unsigned int count, void *data)
{

    struct event_walkrequest *request = data;
    struct event_walkresponse response;

    if (count < sizeof (struct event_walkrequest) || count - sizeof (struct event_walkrequest) < request->length)
        return MESSAGE_FAILED;

    response.id = walkroot(request->parent, request->length, (char *)(request + 1));

    return kernel_place(MESSAGE_ROOT, source, EVENT_WALKRESPONSE, sizeof (struct event_walkresponse), &response);

}

static unsigned int rootoperands_place(unsigned int target, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    switch (event)
    {

    case EVENT_READREQUEST:
        return onrootreadrequest(source, count, data);

    case EVENT_STATREQUEST:
        return onrootstatrequest(source, count, data);

    case EVENT_WALKREQUEST:
        return onrootwalkrequest(source, count, data);

    }

    return MESSAGE_FAILED;

}

void namesrv_setup(void)
{

    server_init(&rootserver, 0, rootoperands_place, 0);
    service_init(&rootservice, "namesrv");
    pool_setroot(&rootserver.resource);
    service_register(&rootservice, MESSAGE_ROOT);

}

