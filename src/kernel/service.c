#include <fudge.h>
#include "resource.h"
#include "service.h"

unsigned int service_findpath(struct service *service, unsigned int id, char *path, unsigned int length)
{

    unsigned int offset = buffer_firstbyte(path, length, ':');

    while (offset < length)
    {

        char *cp = path + offset;
        unsigned int cl = buffer_findbyte(cp, length - offset, '/');

        if (cl == 0)
        {

            id = service->root();

        }

        else if (cl == 2 && cp[0] == '.' && cp[1] == '.')
        {

            if (id != service->root())
                id = service->parent(id);

        }

        else
        {

            id = service->child(id, cp, cl);

        }

        if (!id)
            return 0;

        offset += cl + 1;

    }

    return id;

}

struct service *service_find(unsigned int count, char *name)
{

    struct resource *current = 0;

    while ((current = resource_foreachtype(current, RESOURCE_SERVICE)))
    {

        struct service *service = current->data;

        if (buffer_match(service->name, name, count))
            return service;

    }

    return 0;

}

void service_init(struct service *service, char *name, unsigned int (*root)(void), unsigned int (*parent)(unsigned int id), unsigned int (*child)(unsigned int id, char *path, unsigned int length), unsigned int (*create)(unsigned int id, char *name, unsigned int length), unsigned int (*destroy)(unsigned int id), unsigned int (*stat)(unsigned int id, struct record *record), unsigned int (*list)(unsigned int id, unsigned int offset, unsigned int count, struct record *record), unsigned int (*read)(unsigned int id, void *buffer, unsigned int count, unsigned int offset), unsigned int (*write)(unsigned int id, void *buffer, unsigned int count, unsigned int offset), unsigned int (*map)(unsigned int id), unsigned int (*link)(unsigned int id, unsigned int target, unsigned int source), unsigned int (*unlink)(unsigned int id, unsigned int target), unsigned int (*notify)(unsigned int id, unsigned int source, unsigned int event, unsigned int count, void *data))
{

    resource_init(&service->resource, RESOURCE_SERVICE, service);

    service->name = name;
    service->root = root;
    service->parent = parent;
    service->child = child;
    service->create = create;
    service->destroy = destroy;
    service->stat = stat;
    service->list = list;
    service->read = read;
    service->write = write;
    service->map = map;
    service->link = link;
    service->unlink = unlink;
    service->notify = notify;

}

