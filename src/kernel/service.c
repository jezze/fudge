#include <fudge.h>
#include "resource.h"
#include "binary.h"
#include "service.h"

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

void service_initbackend(struct service_backend *backend, unsigned int id, unsigned int (*read)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(unsigned int offset, unsigned int count, void *buffer), unsigned long (*getphysical)(void))
{

    resource_init(&backend->resource, RESOURCE_SERVICEBACKEND, backend);

    backend->id = id;
    backend->read = read;
    backend->write = write;
    backend->getphysical = getphysical;

}

void service_initprotocol(struct service_protocol *protocol, unsigned int (*match)(struct service_backend *backend), unsigned int (*root)(struct service_backend *backend), unsigned int (*parent)(struct service_backend *backend, unsigned int id), unsigned int (*child)(struct service_backend *backend, unsigned int id, unsigned int count, char *path), unsigned int (*create)(struct service_backend *backend, unsigned int id, unsigned int count, char *name), unsigned int (*destroy)(struct service_backend *backend, unsigned int id, unsigned int count, char *name), unsigned int (*open)(struct service_backend *backend, struct list_item *link, struct service_state *state), unsigned int (*close)(struct service_backend *backend, struct list_item *link, struct service_state *state), unsigned int (*read)(struct service_backend *backend, struct list_item *link, struct service_state *state, unsigned int count, void *buffer), unsigned int (*write)(struct service_backend *backend, struct list_item *link, struct service_state *state, unsigned int count, void *buffer), unsigned int (*seek)(struct service_backend *backend, struct service_state *state, unsigned int offset), unsigned long (*map)(struct service_backend *backend, unsigned int id, struct binary_node *node))
{

    resource_init(&protocol->resource, RESOURCE_SERVICEPROTOCOL, protocol);

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
    protocol->seek = seek;
    protocol->map = map;

}

