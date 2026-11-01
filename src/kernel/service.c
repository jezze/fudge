#include <fudge.h>
#include <hash.h>
#include "resource.h"
#include "service.h"
#include "kernel.h"

#define ROOT                            1
#define ROOTRECORDS                     2

struct listing
{

    struct event_readresponse response;
    struct record records[ROOTRECORDS];

};

struct statresult
{

    struct event_statresponse response;
    struct record record;

};

static void setname(struct service *service, char *name)
{

    buffer_clear(service->name, SERVICE_NAMESIZE);

    if (name)
    {

        buffer_copy(service->name, name, cstring_length_zero(name));

        service->namehash = djb_hash(cstring_length(name), name);

    }

    else
    {

        service->namehash = 0;

    }

}

/* only . and .. lead anywhere: back to the root */
static unsigned int walk(unsigned int parent, unsigned int length, char *path)
{

    unsigned int offset = 0;

    if (parent && parent != ROOT)
        return 0;

    while (offset < length)
    {

        char *cp = path + offset;
        unsigned int cl = buffer_findbyte(cp, length - offset, '/');

        if (cl > 2 || (cl && !buffer_match(cp, "..", cl)))
            return 0;

        offset += cl + 1;

    }

    return ROOT;

}

static unsigned int onreadrequest(struct service *service, unsigned int source, unsigned int count, void *data)
{

    struct event_readrequest *request = data;
    struct listing listing;
    unsigned int c = 0;
    unsigned int i;

    if (count < sizeof (struct event_readrequest))
        return MESSAGE_FAILED;

    /* the record at offset i is named by the first i + 1 characters of "..", its offset is the next one */
    for (i = request->offset; request->id == ROOT && i < ROOTRECORDS && (c + 1) * sizeof (struct record) <= request->count; i++)
        record_init(&listing.records[c++], ROOT, RECORD_TYPE_DIRECTORY, 0, i + 1, i + 1, "..");

    listing.response.count = c * sizeof (struct record);

    return kernel_place(service->inode, source, EVENT_READRESPONSE, sizeof (struct event_readresponse) + listing.response.count, &listing);

}

static unsigned int onstatrequest(struct service *service, unsigned int source, unsigned int count, void *data)
{

    struct event_statrequest *request = data;
    struct statresult result;

    if (count < sizeof (struct event_statrequest))
        return MESSAGE_FAILED;

    record_init(&result.record, ROOT, RECORD_TYPE_DIRECTORY, 0, 1, 1, ".");

    result.response.count = (request->id == ROOT) ? sizeof (struct record) : 0;

    return kernel_place(service->inode, source, EVENT_STATRESPONSE, sizeof (struct event_statresponse) + result.response.count, &result);

}

static unsigned int onwalkrequest(struct service *service, unsigned int source, unsigned int count, void *data)
{

    struct event_walkrequest *request = data;
    struct event_walkresponse response;

    if (count < sizeof (struct event_walkrequest) || count - sizeof (struct event_walkrequest) < request->length)
        return MESSAGE_FAILED;

    response.id = walk(request->parent, request->length, (char *)(request + 1));

    return kernel_place(service->inode, source, EVENT_WALKRESPONSE, sizeof (struct event_walkresponse), &response);

}

/* the filesystem requests every service answers: its root is a directory holding only . and .. */
unsigned int service_place(struct service *service, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    switch (event)
    {

    case EVENT_READREQUEST:
        return onreadrequest(service, source, count, data);

    case EVENT_STATREQUEST:
        return onstatrequest(service, source, count, data);

    case EVENT_WALKREQUEST:
        return onwalkrequest(service, source, count, data);

    }

    return MESSAGE_FAILED;

}

void service_register(struct service *service, unsigned int inode, char *name)
{

    resource_register(&service->resource);
    setname(service, name);

    service->inode = inode;

}

void service_unregister(struct service *service)
{

    resource_unregister(&service->resource);
    setname(service, 0);

    service->inode = 0;

}

void service_init(struct service *service)
{

    resource_init(&service->resource, RESOURCE_SERVICE, service);
    list_init(&service->links);

}

