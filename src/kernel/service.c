#include <fudge.h>
#include "resource.h"
#include "task.h"
#include "service.h"

struct service_backend *service_findbackend(unsigned int id)
{

    struct resource *current = 0;

    while ((current = resource_findtype(current, RESOURCE_VFSBACKEND)))
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

    while ((current = resource_findtype(current, RESOURCE_VFSPROTOCOL)))
    {

        struct service_protocol *protocol = current->data;

        if (protocol->match(backend))
            return protocol;

    }

    return 0;

}

void service_initbackend(struct service_backend *backend, unsigned int id, unsigned int (*read)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(unsigned int offset, unsigned int count, void *buffer), unsigned long (*getphysical)(void))
{

    resource_init(&backend->resource, RESOURCE_VFSBACKEND, backend);

    backend->id = id;
    backend->read = read;
    backend->write = write;
    backend->getphysical = getphysical;

}

void service_initprotocol(struct service_protocol *protocol, unsigned int (*match)(struct service_backend *backend), unsigned int (*root)(struct service_backend *backend), unsigned int (*parent)(struct service_backend *backend, unsigned int id), unsigned int (*child)(struct service_backend *backend, unsigned int id, unsigned int count, char *path), unsigned int (*create)(struct service_backend *backend, unsigned int id, unsigned int count, char *name), unsigned int (*destroy)(struct service_backend *backend, unsigned int id, unsigned int count, char *name), unsigned int (*open)(struct service_backend *backend, struct task *task, unsigned int descriptor, unsigned int id), unsigned int (*close)(struct service_backend *backend, struct task *task, unsigned int descriptor, unsigned int id), unsigned int (*read)(struct service_backend *backend, struct task *task, unsigned int descriptor, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct service_backend *backend, struct task *task, unsigned int descriptor, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*scan)(struct service_backend *backend, unsigned int id, unsigned int index), unsigned long (*getphysical)(struct service_backend *backend, unsigned int id))
{

    resource_init(&protocol->resource, RESOURCE_VFSPROTOCOL, protocol);

    protocol->match = match;
    protocol->root = root;
    protocol->parent = parent;
    protocol->child = child;
    protocol->create = create;
    protocol->destroy = destroy;
    protocol->open = open;
    protocol->close = close;
    protocol->read = read;
    protocol->write = write;
    protocol->scan = scan;
    protocol->getphysical = getphysical;

}

void service_initchannel(struct service_channel *channel)
{

    resource_init(&channel->resource, RESOURCE_VFSCHANNEL, channel);

}

