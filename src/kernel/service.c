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

struct child
{

    unsigned int inode;
    unsigned int index;

};

static struct node_operands operands;

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

static struct child findchild(struct service *parent, unsigned int inode, unsigned int index)
{

    struct resource *resource = 0;
    struct child child;

    child.inode = 0;
    child.index = 0;

    resource_lock();

    while ((resource = resource_foreachtype_unsafe(resource, RESOURCE_SERVICE)))
    {

        struct service *service = resource->data;

        if (service->parent == parent && (inode ? service->inode == inode : service->index == index))
        {

            child.inode = service->inode;
            child.index = service->index;

            break;

        }

    }

    resource_unlock();

    return child;

}

/* a child is named by its index in decimal, so 01 or 1x name nothing */
static unsigned int walkchild(struct service *parent, char *name, unsigned int length)
{

    char current[SERVICE_NAMESIZE];
    struct child child = findchild(parent, 0, cstring_read_value(name, length, 10));

    if (child.inode && cstring_write_value(current, SERVICE_NAMESIZE, child.index, 10, 0, 0) == length && buffer_match(current, name, length))
        return child.inode;

    return 0;

}

static void initchildrecord(struct record *record, struct child *child, unsigned int offset)
{

    char name[SERVICE_NAMESIZE];

    record_init(record, child->inode, RECORD_TYPE_DIRECTORY, 0, offset, cstring_write_value(name, SERVICE_NAMESIZE, child->index, 10, 0, 0), name);

}

/* the root's id is the directory's own inode, a child's id is the child's inode */
static unsigned int walk(struct service *service, unsigned int parent, unsigned int length, char *path)
{

    unsigned int id = (parent) ? parent : service->inode;
    unsigned int offset = 0;

    if (id != service->inode && !findchild(service, id, 0).inode)
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

    if (request->id != service->inode && !findchild(service, request->id, 0).inode)
        last = 0;

    for (i = request->offset; i < last && c < LISTRECORDS && (c + 1) * sizeof (struct record) <= request->count; i++)
    {

        if (i < 2)
        {

            record_init(&listing.records[c++], request->id, RECORD_TYPE_DIRECTORY, 0, i + 1, i + 1, "..");

        }

        else
        {

            struct child child = findchild(service, 0, i - 2);

            if (child.inode)
                initchildrecord(&listing.records[c++], &child, i + 1);

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

        struct child child = findchild(service, request->id, 0);

        if (child.inode)
            initchildrecord(&result.record, &child, 0);
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

void service_register(struct service *service, unsigned int inode, char *name)
{

    resource_register(&service->resource);
    setname(service, name);

    service->inode = inode;
    service->parent = 0;
    service->index = 0;

}

/* a directory of children found by walking their index, as in block:1; its node answers only walk, stat and read */
void service_registerdirectory(struct service *service, char *name)
{

    unsigned int inode;

    node_operands_init(&operands, 0, operands_place, 0);

    inode = pool_picknode(&service->resource, &operands);

    if (inode)
        service_register(service, inode, name);

}

/* takes the lowest free index, so with nothing unregistered children are numbered in the order they come */
void service_registerchild(struct service *service, unsigned int inode, struct service *parent)
{

    struct resource *resource = 0;
    unsigned int used = 0;
    unsigned int index;

    resource_lock();

    while ((resource = resource_foreachtype_unsafe(resource, RESOURCE_SERVICE)))
    {

        struct service *current = resource->data;

        if (current->parent == parent && current->index < SERVICE_CHILDREN)
            used |= 1U << current->index;

    }

    resource_unlock();

    for (index = 0; index < SERVICE_CHILDREN && (used & (1U << index)); index++);

    if (index == SERVICE_CHILDREN)
        return;

    resource_register(&service->resource);
    setname(service, 0);

    service->inode = inode;
    service->parent = parent;
    service->index = index;

}

void service_unregister(struct service *service)
{

    resource_unregister(&service->resource);
    setname(service, 0);

    service->inode = 0;
    service->parent = 0;
    service->index = 0;

}

void service_init(struct service *service)
{

    resource_init(&service->resource, RESOURCE_SERVICE, service);
    list_init(&service->links);

}

