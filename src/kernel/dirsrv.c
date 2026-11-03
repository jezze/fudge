#include <fudge.h>
#include "resource.h"
#include "pool.h"
#include "kernel.h"
#include "dirsrv.h"

#define LISTRECORDS                     8
#define TEXTSIZE                        32

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
    char data[TEXTSIZE];

};

static unsigned int dirid(unsigned int index)
{

    return (index * 2 + 1) * POOL_NODES;

}

static unsigned int fileid(unsigned int index)
{

    return (index * 2 + 2) * POOL_NODES;

}

static struct resource *getdevice(struct dirsrv *dirsrv, unsigned int index)
{

    struct resource *resource = 0;
    unsigned int i = 0;

    resource_lock();

    while ((resource = resource_foreachtype_unsafe(resource, dirsrv->type)) && i++ < index);

    resource_unlock();

    return resource;

}

static struct resource *getdir(struct dirsrv *dirsrv, unsigned int id)
{

    unsigned int index = id / POOL_NODES / 2;

    return (id == dirid(index)) ? getdevice(dirsrv, index) : 0;

}

static struct resource *getfile(struct dirsrv *dirsrv, unsigned int id)
{

    unsigned int index = (id / POOL_NODES - 1) / 2;
    struct resource *resource = (id && id == fileid(index)) ? getdevice(dirsrv, index) : 0;

    return (resource && resource->inode) ? resource : 0;

}

static unsigned int walkchild(struct dirsrv *dirsrv, unsigned int id, char *name, unsigned int length)
{

    if (id == dirsrv->resource.inode)
    {

        char current[TEXTSIZE];
        unsigned int index = cstring_read_value(name, length, 10);

        if (cstring_write_value(current, TEXTSIZE, index, 10, 0, 0) == length && buffer_match(current, name, length) && getdevice(dirsrv, index))
            return dirid(index);

    }

    else
    {

        struct resource *device = getdir(dirsrv, id);

        if (device && device->inode && length == 5 && buffer_match(name, "inode", 5))
            return id + POOL_NODES;

    }

    return 0;

}

static unsigned int walk(struct dirsrv *dirsrv, unsigned int parent, unsigned int length, char *path)
{

    unsigned int id = (parent) ? parent : dirsrv->resource.inode;
    unsigned int offset = 0;

    if (id != dirsrv->resource.inode && !getdir(dirsrv, id) && !getfile(dirsrv, id))
        return 0;

    while (offset < length)
    {

        char *cp = path + offset;
        unsigned int cl = buffer_findbyte(cp, length - offset, '/');

        if (cl == 2 && buffer_match(cp, "..", 2))
            id = (getfile(dirsrv, id)) ? id - POOL_NODES : dirsrv->resource.inode;
        else if (cl && (cl != 1 || !buffer_match(cp, ".", 1)))
            id = walkchild(dirsrv, id, cp, cl);

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

static unsigned int readfile(struct dirsrv *dirsrv, struct resource *device, unsigned int source, struct event_readrequest *request)
{

    char text[TEXTSIZE];
    struct content content;
    unsigned int length = cstring_write_value(text, TEXTSIZE, device->inode, 10, 0, 0);

    content.response.count = (request->offset < length) ? buffer_write(content.data, (request->count < TEXTSIZE) ? request->count : TEXTSIZE, text + request->offset, length - request->offset, 0) : 0;

    return kernel_place(dirsrv->resource.inode, source, EVENT_READRESPONSE, sizeof (struct event_readresponse) + content.response.count, &content);

}

static unsigned int onreadrequest(struct dirsrv *dirsrv, unsigned int source, unsigned int count, void *data)
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

    file = getfile(dirsrv, request->id);

    if (file)
        return readfile(dirsrv, file, source, request);

    dir = getdir(dirsrv, request->id);
    root = request->id == dirsrv->resource.inode;

    for (i = request->offset; (root || dir) && c < LISTRECORDS && (c + 1) * sizeof (struct record) <= request->count; i++)
    {

        if (i < 2)
            record_init(&listing.records[c++], request->id, RECORD_TYPE_DIRECTORY, 0, i + 1, i + 1, "..");
        else if (root && getdevice(dirsrv, i - 2))
            initdirrecord(&listing.records[c++], i - 2, i + 1);
        else if (dir && i == 2 && dir->inode)
            initfilerecord(&listing.records[c++], dir, request->id + POOL_NODES, i + 1);
        else
            break;

    }

    listing.response.count = c * sizeof (struct record);

    return kernel_place(dirsrv->resource.inode, source, EVENT_READRESPONSE, sizeof (struct event_readresponse) + listing.response.count, &listing);

}

static unsigned int onstatrequest(struct dirsrv *dirsrv, unsigned int source, unsigned int count, void *data)
{

    struct event_statrequest *request = data;
    struct resource *device;
    struct statresult result;

    if (count < sizeof (struct event_statrequest))
        return MESSAGE_FAILED;

    result.response.count = sizeof (struct record);

    if (request->id == dirsrv->resource.inode)
        record_init(&result.record, request->id, RECORD_TYPE_DIRECTORY, 0, 1, 1, ".");
    else if (getdir(dirsrv, request->id))
        initdirrecord(&result.record, request->id / POOL_NODES / 2, 0);
    else if ((device = getfile(dirsrv, request->id)))
        initfilerecord(&result.record, device, request->id, 0);
    else
        result.response.count = 0;

    return kernel_place(dirsrv->resource.inode, source, EVENT_STATRESPONSE, sizeof (struct event_statresponse) + result.response.count, &result);

}

static unsigned int onwalkrequest(struct dirsrv *dirsrv, unsigned int source, unsigned int count, void *data)
{

    struct event_walkrequest *request = data;
    struct event_walkresponse response;

    if (count < sizeof (struct event_walkrequest) || count - sizeof (struct event_walkrequest) < request->length)
        return MESSAGE_FAILED;

    response.id = walk(dirsrv, request->parent, request->length, (char *)(request + 1));

    return kernel_place(dirsrv->resource.inode, source, EVENT_WALKRESPONSE, sizeof (struct event_walkresponse), &response);

}

static unsigned int oncreaterequest(struct dirsrv *dirsrv, unsigned int source, unsigned int count, void *data)
{

    struct event_createrequest *request = data;
    struct event_createresponse response;
    struct resource *device;

    if (count < sizeof (struct event_createrequest) || count - sizeof (struct event_createrequest) < request->count)
        return MESSAGE_FAILED;

    device = getdir(dirsrv, request->parent);
    response.id = (device && request->count == 5 && buffer_match(request + 1, "inode", 5) && pool_attachnode(device)) ? request->parent + POOL_NODES : 0;

    return kernel_place(dirsrv->resource.inode, source, EVENT_CREATERESPONSE, sizeof (struct event_createresponse), &response);

}

static unsigned int onremoverequest(struct dirsrv *dirsrv, unsigned int source, unsigned int count, void *data)
{

    struct event_removerequest *request = data;
    struct event_removeresponse response;
    struct resource *device;

    if (count < sizeof (struct event_removerequest))
        return MESSAGE_FAILED;

    device = getfile(dirsrv, request->id);
    response.status = (device && request->parent == request->id - POOL_NODES);

    if (response.status)
        pool_detachnode(device);

    return kernel_place(dirsrv->resource.inode, source, EVENT_REMOVERESPONSE, sizeof (struct event_removeresponse), &response);

}

static unsigned int operands_place(unsigned int target, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    struct resource *resource = pool_getnoderesource(target);

    if (resource)
    {

        struct dirsrv *dirsrv = resource->data;

        switch (event)
        {

        case EVENT_CREATEREQUEST:
            return oncreaterequest(dirsrv, source, count, data);

        case EVENT_READREQUEST:
            return onreadrequest(dirsrv, source, count, data);

        case EVENT_REMOVEREQUEST:
            return onremoverequest(dirsrv, source, count, data);

        case EVENT_STATREQUEST:
            return onstatrequest(dirsrv, source, count, data);

        case EVENT_WALKREQUEST:
            return onwalkrequest(dirsrv, source, count, data);

        }

    }

    return MESSAGE_FAILED;

}

void dirsrv_init(struct dirsrv *dirsrv, unsigned int type)
{

    resource_operands_init(&dirsrv->operands, 0, operands_place, 0);
    resource_init(&dirsrv->resource, 0, dirsrv, &dirsrv->operands);

    dirsrv->type = type;

}

