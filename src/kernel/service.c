#include <fudge.h>
#include <hash.h>
#include "resource.h"
#include "node.h"
#include "pool.h"
#include "service.h"
#include "kernel.h"

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

static struct node_operands operands;
static struct node_operands rootoperands;

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

/* the index of the child with this inode, SERVICE_CHILDREN if none; inode 0 finds the first free index */
static unsigned int findchild(struct service *service, unsigned int inode)
{

    unsigned int index;

    for (index = 0; index < SERVICE_CHILDREN && service->children[index] != inode; index++);

    return index;

}

/* a child is named by its index in decimal, so 01 or 1x name nothing */
static unsigned int walkchild(struct service *service, char *name, unsigned int length)
{

    char current[SERVICE_NAMESIZE];
    unsigned int index = cstring_read_value(name, length, 10);

    if (index < SERVICE_CHILDREN && service->children[index] && cstring_write_value(current, SERVICE_NAMESIZE, index, 10, 0, 0) == length && buffer_match(current, name, length))
        return service->children[index];

    return 0;

}

static void initchildrecord(struct record *record, struct service *service, unsigned int index, unsigned int offset)
{

    char name[SERVICE_NAMESIZE];

    record_init(record, service->children[index], RECORD_TYPE_DIRECTORY, 0, offset, cstring_write_value(name, SERVICE_NAMESIZE, index, 10, 0, 0), name);

}

/* the root's id is the directory's own inode, a child's id is the child's inode */
static unsigned int walk(struct service *service, unsigned int parent, unsigned int length, char *path)
{

    unsigned int id = (parent) ? parent : service->inode;
    unsigned int offset = 0;

    if (id != service->inode && findchild(service, id) == SERVICE_CHILDREN)
        return 0;

    while (offset < length)
    {

        char *cp = path + offset;
        unsigned int cl = buffer_findbyte(cp, length - offset, '/');

        if (cl == 2 && buffer_match(cp, "..", 2))
            id = service->inode;
        else if (cl && (cl != 1 || !buffer_match(cp, ".", 1)))
            id = (id == service->inode) ? walkchild(service, cp, cl) : 0;

        if (!id)
            return 0;

        offset += cl + 1;

    }

    return id;

}

/* the root lists . and .. at offsets 0 and 1 and the child with index i at offset i + 2; a child lists only . and .. */
static unsigned int onreadrequest(struct service *service, unsigned int source, unsigned int count, void *data)
{

    struct event_readrequest *request = data;
    unsigned int last = (request->id == service->inode) ? SERVICE_CHILDREN + 2 : 2;
    struct listing listing;
    unsigned int c = 0;
    unsigned int i;

    if (count < sizeof (struct event_readrequest))
        return MESSAGE_FAILED;

    if (request->id != service->inode && (!request->id || findchild(service, request->id) == SERVICE_CHILDREN))
        last = 0;

    for (i = request->offset; i < last && c < LISTRECORDS && (c + 1) * sizeof (struct record) <= request->count; i++)
    {

        if (i < 2)
        {

            record_init(&listing.records[c++], request->id, RECORD_TYPE_DIRECTORY, 0, i + 1, i + 1, "..");

        }

        else if (service->children[i - 2])
        {

            initchildrecord(&listing.records[c++], service, i - 2, i + 1);

        }

    }

    listing.response.count = c * sizeof (struct record);

    return kernel_place(service->inode, source, EVENT_READRESPONSE, sizeof (struct event_readresponse) + listing.response.count, &listing);

}

static unsigned int onstatrequest(struct service *service, unsigned int source, unsigned int count, void *data)
{

    struct event_statrequest *request = data;
    struct statresult result;

    if (count < sizeof (struct event_statrequest))
        return MESSAGE_FAILED;

    result.response.count = sizeof (struct record);

    if (request->id == service->inode)
    {

        record_init(&result.record, request->id, RECORD_TYPE_DIRECTORY, 0, 1, 1, ".");

    }

    else
    {

        unsigned int index = findchild(service, request->id);

        if (request->id && index < SERVICE_CHILDREN)
            initchildrecord(&result.record, service, index, 0);
        else
            result.response.count = 0;

    }

    return kernel_place(service->inode, source, EVENT_STATRESPONSE, sizeof (struct event_statresponse) + result.response.count, &result);

}

static unsigned int onwalkrequest(struct service *service, unsigned int source, unsigned int count, void *data)
{

    struct event_walkrequest *request = data;
    struct event_walkresponse response;

    if (count < sizeof (struct event_walkrequest) || count - sizeof (struct event_walkrequest) < request->length)
        return MESSAGE_FAILED;

    response.id = walk(service, request->parent, request->length, (char *)(request + 1));

    return kernel_place(service->inode, source, EVENT_WALKRESPONSE, sizeof (struct event_walkresponse), &response);

}

static unsigned int operands_place(unsigned int target, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    struct resource *resource = pool_getnoderesource(target);

    if (resource)
    {

        struct service *service = resource->data;

        switch (event)
        {

        case EVENT_READREQUEST:
            return onreadrequest(service, source, count, data);

        case EVENT_STATREQUEST:
            return onstatrequest(service, source, count, data);

        case EVENT_WALKREQUEST:
            return onwalkrequest(service, source, count, data);

        }

    }

    return MESSAGE_FAILED;

}

/* the inode of the named service, 0 if there is none */
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

/* the root knows only the services' names: what is inside a service is the service's own */
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

/* . and .. at offsets 0 and 1, then the services in the order they were registered */
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

void service_register(struct service *service, unsigned int inode, char *name)
{

    resource_register(&service->resource);
    setname(service, name);

    service->inode = inode;

}

/* a directory of children found by walking their index, as in block:1; its node answers only walk, stat and read */
void service_registerdirectory(struct service *service, char *name)
{

    unsigned int inode = pool_picknode(&service->resource, &operands);

    if (inode)
        service_register(service, inode, name);

}

/* takes the lowest free index, so with nothing removed children are numbered in the order they come */
void service_addchild(struct service *service, unsigned int inode)
{

    unsigned int index = findchild(service, 0);

    if (index < SERVICE_CHILDREN)
        service->children[index] = inode;

}

void service_removechild(struct service *service, unsigned int inode)
{

    unsigned int index = findchild(service, inode);

    if (inode && index < SERVICE_CHILDREN)
        service->children[index] = 0;

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
    buffer_clear(service->children, SERVICE_CHILDREN * sizeof (unsigned int));

}

/* the root at MESSAGE_ROOT is where programs start: walking a service's name gives its inode */
void service_setup(void)
{

    node_operands_init(&operands, 0, operands_place, 0);
    node_operands_init(&rootoperands, 0, rootoperands_place, 0);
    pool_setroot(&rootoperands);

}

