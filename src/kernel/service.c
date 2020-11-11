#include <fudge.h>
#include "resource.h"
#include "service.h"

struct service_backend *service_findbackend(unsigned int id)
{

    struct resource *current = 0;

    while ((current = resource_foreachtype(current, RESOURCE_SERVICEBACKEND)))
    {

        struct service_backend *backend = current->data;

        if (backend->id == id)
            return backend;

    }

    return 0;

}

struct service_protocol *service_findprotocol(unsigned int id)
{

    struct resource *current = 0;

    while ((current = resource_foreachtype(current, RESOURCE_SERVICEPROTOCOL)))
    {

        struct service_protocol *protocol = current->data;

        if (protocol->id == id)
            return protocol;

    }

    return 0;

}

void service_initstate(struct service_state *state, unsigned int id)
{

    list_inititem(&state->item, state);

    state->id = id;

}

void service_initbackend(struct service_backend *backend, unsigned int id, unsigned int (*read)(void *buffer, unsigned int count, unsigned int offset), unsigned int (*write)(void *buffer, unsigned int count, unsigned int offset), unsigned int (*map)(unsigned int offset, unsigned int count))
{

    resource_init(&backend->resource, RESOURCE_SERVICEBACKEND, backend);

    backend->id = id;
    backend->read = read;
    backend->write = write;
    backend->map = map;

}

void service_initprotocol(struct service_protocol *protocol, unsigned int id, unsigned int (*root)(struct service_backend *backend), unsigned int (*parent)(struct service_backend *backend, unsigned int id), unsigned int (*child)(struct service_backend *backend, unsigned int id, char *path, unsigned int length), unsigned int (*create)(struct service_backend *backend, unsigned int id, char *name, unsigned int length), unsigned int (*destroy)(struct service_backend *backend, unsigned int id, char *name, unsigned int length), unsigned int (*step)(struct service_backend *backend, unsigned int id, unsigned int current), unsigned int (*open)(struct service_backend *backend, struct service_state *state, unsigned int id), unsigned int (*close)(struct service_backend *backend, struct service_state *state, unsigned int id), unsigned int (*read)(struct service_backend *backend, struct service_state *state, unsigned int id, unsigned int current, void *buffer, unsigned int count, unsigned int offset), unsigned int (*write)(struct service_backend *backend, struct service_state *state, unsigned int id, unsigned int current, void *buffer, unsigned int count, unsigned int offset), unsigned int (*seek)(struct service_backend *backend, unsigned int id, unsigned int offset), unsigned int (*map)(struct service_backend *backend, unsigned int id))
{

    resource_init(&protocol->resource, RESOURCE_SERVICEPROTOCOL, protocol);

    protocol->id = id;
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

void service_initdescriptor(struct service_descriptor *descriptor, unsigned int id)
{

    service_initstate(&descriptor->state, id);

}

void service_initmount(struct service_mount *mount)
{

    list_inititem(&mount->item, mount);

}

