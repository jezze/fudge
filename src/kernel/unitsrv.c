#include <fudge.h>
#include "resource.h"
#include "pool.h"
#include "server.h"
#include "kernel.h"
#include "unitsrv.h"

#define LISTRECORDS                     8
#define TEXTSIZE                        32

static unsigned int dirid(unsigned int index)
{

    return (index * 2 + 1) * POOL_NODES;

}

static unsigned int fileid(unsigned int index)
{

    return (index * 2 + 2) * POOL_NODES;

}

static struct resource *getdevice(struct unitsrv *unitsrv, unsigned int index)
{

    struct resource *resource = 0;
    unsigned int i = 0;

    resource_lock();

    while ((resource = resource_foreachtype_unsafe(resource, unitsrv->type)) && i++ < index);

    resource_unlock();

    return resource;

}

static struct resource *getdir(struct unitsrv *unitsrv, unsigned int id)
{

    unsigned int index = id / POOL_NODES / 2;

    return (id == dirid(index)) ? getdevice(unitsrv, index) : 0;

}

static struct resource *getfile(struct unitsrv *unitsrv, unsigned int id)
{

    unsigned int index = (id / POOL_NODES - 1) / 2;
    struct resource *resource = (id && id == fileid(index)) ? getdevice(unitsrv, index) : 0;

    return (resource && resource->inode) ? resource : 0;

}

static unsigned int walkchild(struct unitsrv *unitsrv, unsigned int id, char *name, unsigned int length)
{

    if (id == unitsrv->server.resource.inode)
    {

        char current[TEXTSIZE];
        unsigned int index = cstring_read_value(name, length, 10);

        if (cstring_write_value(current, TEXTSIZE, index, 10, 0, 0) == length && buffer_match(current, name, length) && getdevice(unitsrv, index))
            return dirid(index);

    }

    else
    {

        struct resource *device = getdir(unitsrv, id);

        if (device && device->inode && length == 5 && buffer_match(name, "inode", 5))
            return id + POOL_NODES;

    }

    return 0;

}

static unsigned int walk(struct unitsrv *unitsrv, unsigned int parent, unsigned int length, char *path)
{

    unsigned int id = (parent) ? parent : unitsrv->server.resource.inode;
    unsigned int offset = 0;

    if (id != unitsrv->server.resource.inode && !getdir(unitsrv, id) && !getfile(unitsrv, id))
        return 0;

    while (offset < length)
    {

        char *cp = path + offset;
        unsigned int cl = buffer_findbyte(cp, length - offset, '/');

        if (cl == 2 && buffer_match(cp, "..", 2))
            id = (getfile(unitsrv, id)) ? id - POOL_NODES : unitsrv->server.resource.inode;
        else if (cl && (cl != 1 || !buffer_match(cp, ".", 1)))
            id = walkchild(unitsrv, id, cp, cl);

        if (!id)
            return 0;

        offset += cl + 1;

    }

    return id;

}

static void initdirrecord(struct record *record, unsigned int index, unsigned int offset)
{

    char name[TEXTSIZE];

    record_init(record, dirid(index), RECORD_TYPE_DIRECTORY, 0, offset, cstring_write_value(name, TEXTSIZE, index, 10, 0, 0), name);

}

static void initfilerecord(struct record *record, struct resource *device, unsigned int id, unsigned int offset)
{

    char text[TEXTSIZE];

    record_init(record, id, RECORD_TYPE_NORMAL, cstring_write_value(text, TEXTSIZE, device->inode, 10, 0, 0), offset, 5, "inode");

}

static unsigned int readfile(struct unitsrv *unitsrv, struct resource *device, unsigned int source, struct event_readrequest *request)
{

    unsigned char buffer[sizeof (struct event_readresponse) + TEXTSIZE];
    struct event_readresponse *response = (struct event_readresponse *)buffer;
    char text[TEXTSIZE];
    unsigned int length = cstring_write_value(text, TEXTSIZE, device->inode, 10, 0, 0);

    response->count = (request->offset < length) ? buffer_write(response + 1, (request->count < TEXTSIZE) ? request->count : TEXTSIZE, text + request->offset, length - request->offset, 0) : 0;
    response->buffer = 0;

    return kernel_place(unitsrv->server.resource.inode, source, EVENT_READRESPONSE, sizeof (struct event_readresponse) + response->count, buffer);

}

static unsigned int onreadrequest(struct unitsrv *unitsrv, unsigned int source, unsigned int count, void *data)
{

    struct event_readrequest *request = data;
    struct resource *file;
    struct resource *dir;
    unsigned int root;
    unsigned char buffer[sizeof (struct event_readresponse) + sizeof (struct record) * LISTRECORDS];
    struct event_readresponse *response = (struct event_readresponse *)buffer;
    struct record *records = (struct record *)(response + 1);
    unsigned int c = 0;
    unsigned int i;

    if (count < sizeof (struct event_readrequest))
        return MESSAGE_FAILED;

    file = getfile(unitsrv, request->id);

    if (file)
        return readfile(unitsrv, file, source, request);

    dir = getdir(unitsrv, request->id);
    root = request->id == unitsrv->server.resource.inode;

    for (i = request->offset; (root || dir) && c < LISTRECORDS && (c + 1) * sizeof (struct record) <= request->count; i++)
    {

        if (i < 2)
            record_init(&records[c++], request->id, RECORD_TYPE_DIRECTORY, 0, i + 1, i + 1, "..");
        else if (root && getdevice(unitsrv, i - 2))
            initdirrecord(&records[c++], i - 2, i + 1);
        else if (dir && i == 2 && dir->inode)
            initfilerecord(&records[c++], dir, request->id + POOL_NODES, i + 1);
        else
            break;

    }

    response->count = c * sizeof (struct record);
    response->buffer = 0;

    return kernel_place(unitsrv->server.resource.inode, source, EVENT_READRESPONSE, sizeof (struct event_readresponse) + response->count, buffer);

}

static unsigned int onstatrequest(struct unitsrv *unitsrv, unsigned int source, unsigned int count, void *data)
{

    struct event_statrequest *request = data;
    struct resource *device;
    unsigned char buffer[sizeof (struct event_statresponse) + sizeof (struct record)];
    struct event_statresponse *response = (struct event_statresponse *)buffer;
    struct record *record = (struct record *)(response + 1);

    if (count < sizeof (struct event_statrequest))
        return MESSAGE_FAILED;

    response->count = sizeof (struct record);

    if (request->id == unitsrv->server.resource.inode)
        record_init(record, request->id, RECORD_TYPE_DIRECTORY, 0, 1, 1, ".");
    else if (getdir(unitsrv, request->id))
        initdirrecord(record, request->id / POOL_NODES / 2, 0);
    else if ((device = getfile(unitsrv, request->id)))
        initfilerecord(record, device, request->id, 0);
    else
        response->count = 0;

    return kernel_place(unitsrv->server.resource.inode, source, EVENT_STATRESPONSE, sizeof (struct event_statresponse) + response->count, buffer);

}

static unsigned int onwalkrequest(struct unitsrv *unitsrv, unsigned int source, unsigned int count, void *data)
{

    struct event_walkrequest *request = data;
    struct event_walkresponse response;

    if (count < sizeof (struct event_walkrequest) || count - sizeof (struct event_walkrequest) < request->length)
        return MESSAGE_FAILED;

    response.id = walk(unitsrv, request->parent, request->length, (char *)(request + 1));

    return kernel_place(unitsrv->server.resource.inode, source, EVENT_WALKRESPONSE, sizeof (struct event_walkresponse), &response);

}

static unsigned int oncreaterequest(struct unitsrv *unitsrv, unsigned int source, unsigned int count, void *data)
{

    struct event_createrequest *request = data;
    struct event_createresponse response;
    struct resource *device;

    if (count < sizeof (struct event_createrequest) || count - sizeof (struct event_createrequest) < request->count)
        return MESSAGE_FAILED;

    device = getdir(unitsrv, request->parent);
    response.id = (device && request->count == 5 && buffer_match(request + 1, "inode", 5) && pool_attachnode(device)) ? request->parent + POOL_NODES : 0;

    return kernel_place(unitsrv->server.resource.inode, source, EVENT_CREATERESPONSE, sizeof (struct event_createresponse), &response);

}

static unsigned int onremoverequest(struct unitsrv *unitsrv, unsigned int source, unsigned int count, void *data)
{

    struct event_removerequest *request = data;
    struct event_removeresponse response;
    struct resource *device;

    if (count < sizeof (struct event_removerequest))
        return MESSAGE_FAILED;

    device = getfile(unitsrv, request->id);
    response.status = (device && request->parent == request->id - POOL_NODES);

    if (response.status)
        pool_detachnode(device);

    return kernel_place(unitsrv->server.resource.inode, source, EVENT_REMOVERESPONSE, sizeof (struct event_removeresponse), &response);

}

static unsigned int operands_place(unsigned int target, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    struct resource *resource = pool_getnoderesource(target);

    if (resource)
    {

        struct unitsrv *unitsrv = resource->data;

        switch (event)
        {

        case EVENT_CREATEREQUEST:
            return oncreaterequest(unitsrv, source, count, data);

        case EVENT_READREQUEST:
            return onreadrequest(unitsrv, source, count, data);

        case EVENT_REMOVEREQUEST:
            return onremoverequest(unitsrv, source, count, data);

        case EVENT_STATREQUEST:
            return onstatrequest(unitsrv, source, count, data);

        case EVENT_WALKREQUEST:
            return onwalkrequest(unitsrv, source, count, data);

        }

    }

    return MESSAGE_FAILED;

}

void unitsrv_init(struct unitsrv *unitsrv, unsigned int type)
{

    server_init(&unitsrv->server, unitsrv, operands_place, 0);

    unitsrv->type = type;

}

