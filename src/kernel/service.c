#include <fudge.h>
#include "resource.h"
#include "binary.h"
#include "container.h"
#include "task.h"
#include "service.h"

static struct service_descriptor descriptors[SERVICE_DESCRIPTORS];

struct service_descriptor *service_getdescriptor(struct task *task, unsigned int descriptor)
{

    return &descriptors[task->id * TASK_DESCRIPTORS + (descriptor & (TASK_DESCRIPTORS - 1))];

}

struct service_backend *service_findbackend(unsigned int id)
{

    struct resource *current = 0;

    while ((current = resource_findtype(current, RESOURCE_SERVICEBACKEND)))
    {

        struct service_backend *backend = current->data;

        if (backend->id == id)
            return backend;

    }

    return 0;

}

struct service_protocol *service_findprotocol(struct service_backend *backend)
{

    struct resource *current = 0;

    while ((current = resource_findtype(current, RESOURCE_SERVICEPROTOCOL)))
    {

        struct service_protocol *protocol = current->data;

        if (protocol->match(backend))
            return protocol;

    }

    return 0;

}

void service_initbackend(struct service_backend *backend, unsigned int id, unsigned int (*read)(void *buffer, unsigned int count, unsigned int offset), unsigned int (*write)(void *buffer, unsigned int count, unsigned int offset), unsigned int (*map)(unsigned int offset, unsigned int count))
{

    resource_init(&backend->resource, RESOURCE_SERVICEBACKEND, backend);

    backend->id = id;
    backend->read = read;
    backend->write = write;
    backend->map = map;

}

void service_initdescriptor(struct service_descriptor *descriptor, struct task *task)
{

    list_inititem(&descriptor->link, descriptor);

    descriptor->task = task;
    descriptor->server = 0;
    descriptor->id = 0;
    descriptor->offset = 0;
    descriptor->current = 0;

}

void service_initprotocol(struct service_protocol *protocol, unsigned int (*match)(struct service_backend *backend), unsigned int (*root)(struct service_backend *backend), unsigned int (*parent)(struct service_descriptor *descriptor), unsigned int (*child)(struct service_descriptor *descriptor, char *path, unsigned int length), unsigned int (*create)(struct service_descriptor *descriptor, char *name, unsigned int length), unsigned int (*destroy)(struct service_descriptor *descriptor, char *name, unsigned int length), unsigned int (*step)(struct service_descriptor *descriptor), unsigned int (*open)(struct service_descriptor *descriptor), unsigned int (*close)(struct service_descriptor *descriptor), unsigned int (*read)(struct service_descriptor *descriptor, void *buffer, unsigned int count), unsigned int (*write)(struct service_descriptor *descriptor, void *buffer, unsigned int count), unsigned int (*seek)(struct service_descriptor *descriptor, unsigned int offset), unsigned int (*map)(struct service_descriptor *descriptor))
{

    resource_init(&protocol->resource, RESOURCE_SERVICEPROTOCOL, protocol);

    protocol->match = match;
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

}

