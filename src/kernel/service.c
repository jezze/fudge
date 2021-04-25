#include <fudge.h>
#include "resource.h"
#include "service.h"

struct service_protocol *service_findprotocol(unsigned int count, char *name)
{

    struct resource *current = 0;

    while ((current = resource_foreachtype(current, RESOURCE_SERVICEPROTOCOL)))
    {

        struct service_protocol *protocol = current->data;

        if (buffer_match(protocol->name, name, count))
            return protocol;

    }

    return 0;

}

void service_initlink(struct service_link *link, unsigned int id)
{

    list_inititem(&link->item, link);

    link->id = id;

}

void service_initprotocol(struct service_protocol *protocol, char *name, unsigned int (*root)(void), unsigned int (*parent)(unsigned int id), unsigned int (*child)(unsigned int id, char *path, unsigned int length), unsigned int (*create)(unsigned int id, char *name, unsigned int length), unsigned int (*destroy)(unsigned int id, char *name, unsigned int length), unsigned int (*step)(unsigned int id, unsigned int current), unsigned int (*open)(unsigned int id), unsigned int (*close)(unsigned int id), unsigned int (*read)(unsigned int id, unsigned int current, void *buffer, unsigned int count, unsigned int offset), unsigned int (*write)(unsigned int id, unsigned int current, void *buffer, unsigned int count, unsigned int offset), unsigned int (*seek)(unsigned int id, unsigned int offset), unsigned int (*map)(unsigned int id), unsigned int (*link)(unsigned int id, struct service_link *link), unsigned int (*unlink)(unsigned int id, struct service_link *link), unsigned int (*notify)(unsigned int id, unsigned int source, struct message_header *header, void *data))
{

    resource_init(&protocol->resource, RESOURCE_SERVICEPROTOCOL, protocol);

    protocol->name = name;
    protocol->root = root;
    protocol->parent = parent;
    protocol->child = child;
    protocol->create = create;
    protocol->destroy = destroy;
    protocol->step = step;
    protocol->open = open;
    protocol->close = close;
    protocol->read = read;
    protocol->write = write;
    protocol->seek = seek;
    protocol->map = map;
    protocol->link = link;
    protocol->unlink = unlink;
    protocol->notify = notify;

}

void service_initdescriptor(struct service_descriptor *descriptor, unsigned int id)
{

    service_initlink(&descriptor->link, id);

    descriptor->protocol = 0;
    descriptor->id = 0;
    descriptor->offset = 0;
    descriptor->current = 0;
    descriptor->count = 0;

}

