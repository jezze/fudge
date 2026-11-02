#include <fudge.h>
#include <hash.h>
#include "resource.h"
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

struct content
{

    struct event_readresponse response;
    char data[SERVICE_NAMESIZE];

};

static struct resource_operands directoryoperands;
static struct resource_operands rootoperands;
static struct resource rootresource;

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

/* a device's directory and its inode file have ids that are multiples of POOL_NODES, which no inode is */
static unsigned int dirid(unsigned int index)
{

    return (index * 2 + 1) * POOL_NODES;

}

static unsigned int fileid(unsigned int index)
{

    return (index * 2 + 2) * POOL_NODES;

}

/* devices are the registered resources of the service's type, numbered in order */
static struct resource *getdevice(struct service *service, unsigned int index)
{

    struct resource *resource = 0;
    unsigned int i = 0;

    resource_lock();

    while ((resource = resource_foreachtype_unsafe(resource, service->type)) && i++ < index);

    resource_unlock();

    return resource;

}

static struct resource *getdir(struct service *service, unsigned int id)
{

    unsigned int index = id / POOL_NODES / 2;

    return (service->type && id == dirid(index)) ? getdevice(service, index) : 0;

}

static struct resource *getfile(struct service *service, unsigned int id)
{

    unsigned int index = (id / POOL_NODES - 1) / 2;
    struct resource *resource = (service->type && id && id == fileid(index)) ? getdevice(service, index) : 0;

    return (resource && resource->inode) ? resource : 0;

}

static unsigned int walkchild(struct service *service, unsigned int id, char *name, unsigned int length)
{

    if (id == service->inode)
    {

        char current[SERVICE_NAMESIZE];
        unsigned int index = cstring_read_value(name, length, 10);

        if (cstring_write_value(current, SERVICE_NAMESIZE, index, 10, 0, 0) == length && buffer_match(current, name, length) && getdevice(service, index))
            return dirid(index);

    }

    else
    {

        struct resource *device = getdir(service, id);

        if (device && device->inode && length == 5 && buffer_match(name, "inode", 5))
            return id + POOL_NODES;

    }

    return 0;

}

static unsigned int walk(struct service *service, unsigned int parent, unsigned int length, char *path)
{

    unsigned int id = (parent) ? parent : service->inode;
    unsigned int offset = 0;

    if (id != service->inode && !getdir(service, id) && !getfile(service, id))
        return 0;

    while (offset < length)
    {

        char *cp = path + offset;
        unsigned int cl = buffer_findbyte(cp, length - offset, '/');

        if (cl == 2 && buffer_match(cp, "..", 2))
            id = (getfile(service, id)) ? id - POOL_NODES : service->inode;
        else if (cl && (cl != 1 || !buffer_match(cp, ".", 1)))
            id = walkchild(service, id, cp, cl);

        if (!id)
            return 0;

        offset += cl + 1;

    }

    return id;

}

static void initdirrecord(struct record *record, unsigned int index, unsigned int offset)
{

    char name[SERVICE_NAMESIZE];

    record_init(record, dirid(index), RECORD_TYPE_DIRECTORY, 0, offset, cstring_write_value(name, SERVICE_NAMESIZE, index, 10, 0, 0), name);

}

static void initfilerecord(struct record *record, struct resource *device, unsigned int id, unsigned int offset)
{

    char text[SERVICE_NAMESIZE];

    record_init(record, id, RECORD_TYPE_NORMAL, cstring_write_value(text, SERVICE_NAMESIZE, device->inode, 10, 0, 0), offset, 5, "inode");

}

static unsigned int readfile(struct service *service, struct resource *device, unsigned int source, struct event_readrequest *request)
{

    char text[SERVICE_NAMESIZE];
    struct content content;
    unsigned int length = cstring_write_value(text, SERVICE_NAMESIZE, device->inode, 10, 0, 0);

    content.response.count = (request->offset < length) ? buffer_write(content.data, (request->count < SERVICE_NAMESIZE) ? request->count : SERVICE_NAMESIZE, text + request->offset, length - request->offset, 0) : 0;

    return kernel_place(service->inode, source, EVENT_READRESPONSE, sizeof (struct event_readresponse) + content.response.count, &content);

}

static unsigned int onreadrequest(struct service *service, unsigned int source, unsigned int count, void *data)
{

    struct event_readrequest *request = data;
    struct resource *file;
    struct resource *dir;
    unsigned int root;
    struct listing listing;
    unsigned int c = 0;
    unsigned int i;

    if (count < sizeof (struct event_readrequest))
        return MESSAGE_FAILED;

    file = getfile(service, request->id);

    if (file)
        return readfile(service, file, source, request);

    dir = getdir(service, request->id);
    root = request->id == service->inode;

    for (i = request->offset; (root || dir) && c < LISTRECORDS && (c + 1) * sizeof (struct record) <= request->count; i++)
    {

        if (i < 2)
            record_init(&listing.records[c++], request->id, RECORD_TYPE_DIRECTORY, 0, i + 1, i + 1, "..");
        else if (root && getdevice(service, i - 2))
            initdirrecord(&listing.records[c++], i - 2, i + 1);
        else if (dir && i == 2 && dir->inode)
            initfilerecord(&listing.records[c++], dir, request->id + POOL_NODES, i + 1);
        else
            break;

    }

    listing.response.count = c * sizeof (struct record);

    return kernel_place(service->inode, source, EVENT_READRESPONSE, sizeof (struct event_readresponse) + listing.response.count, &listing);

}

static unsigned int onstatrequest(struct service *service, unsigned int source, unsigned int count, void *data)
{

    struct event_statrequest *request = data;
    struct resource *device;
    struct statresult result;

    if (count < sizeof (struct event_statrequest))
        return MESSAGE_FAILED;

    result.response.count = sizeof (struct record);

    if (request->id == service->inode)
        record_init(&result.record, request->id, RECORD_TYPE_DIRECTORY, 0, 1, 1, ".");
    else if (getdir(service, request->id))
        initdirrecord(&result.record, request->id / POOL_NODES / 2, 0);
    else if ((device = getfile(service, request->id)))
        initfilerecord(&result.record, device, request->id, 0);
    else
        result.response.count = 0;

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

/* creating a device's inode file attaches a node to the device, removing it detaches the node and the device's links */
static unsigned int oncreaterequest(struct service *service, unsigned int source, unsigned int count, void *data)
{

    struct event_createrequest *request = data;
    struct event_createresponse response;
    struct resource *device;

    if (count < sizeof (struct event_createrequest) || count - sizeof (struct event_createrequest) < request->count)
        return MESSAGE_FAILED;

    device = getdir(service, request->parent);
    response.id = (device && request->count == 5 && buffer_match(request + 1, "inode", 5) && pool_attachnode(device)) ? request->parent + POOL_NODES : 0;

    return kernel_place(service->inode, source, EVENT_CREATERESPONSE, sizeof (struct event_createresponse), &response);

}

static unsigned int onremoverequest(struct service *service, unsigned int source, unsigned int count, void *data)
{

    struct event_removerequest *request = data;
    struct event_removeresponse response;
    struct resource *device;

    if (count < sizeof (struct event_removerequest))
        return MESSAGE_FAILED;

    device = getfile(service, request->id);
    response.status = (device && request->parent == request->id - POOL_NODES);

    if (response.status)
        pool_detachnode(device);

    return kernel_place(service->inode, source, EVENT_REMOVERESPONSE, sizeof (struct event_removeresponse), &response);

}

static unsigned int operands_place(unsigned int target, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    struct resource *resource = pool_getnoderesource(target);

    if (resource)
    {

        struct service *service = resource->data;

        switch (event)
        {

        case EVENT_CREATEREQUEST:
            return oncreaterequest(service, source, count, data);

        case EVENT_READREQUEST:
            return onreadrequest(service, source, count, data);

        case EVENT_REMOVEREQUEST:
            return onremoverequest(service, source, count, data);

        case EVENT_STATREQUEST:
            return onstatrequest(service, source, count, data);

        case EVENT_WALKREQUEST:
            return onwalkrequest(service, source, count, data);

        }

    }

    return MESSAGE_FAILED;

}

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

/* a service of the kernel's own: the node is the service's own resource's */
void service_registernode(struct service *service, char *name, struct resource_operands *operands)
{

    unsigned int inode;

    service->resource.operands = operands;
    inode = pool_attachnode(&service->resource);

    if (inode)
        service_register(service, inode, name);

}

void service_registerdirectory(struct service *service, char *name, unsigned int type)
{

    service->type = type;

    service_registernode(service, name, &directoryoperands);

}

void service_unregister(struct service *service)
{

    if (service->inode)
    {

        resource_unregister(&service->resource);
        setname(service, 0);

        service->inode = 0;

    }

}

void service_init(struct service *service)
{

    resource_init(&service->resource, RESOURCE_SERVICE, service, 0);

    service->type = 0;

}

void service_setup(void)
{

    resource_operands_init(&directoryoperands, 0, operands_place, 0);
    resource_operands_init(&rootoperands, 0, rootoperands_place, 0);
    resource_init(&rootresource, 0, 0, &rootoperands);
    pool_setroot(&rootresource);

}

