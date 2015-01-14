#include <kernel.h>
#include "resource.h"
#include "vfs.h"

struct vfs_backend *vfs_findbackend(unsigned int id)
{

    struct resource *current = 0;

    while ((current = resource_findtype(current, RESOURCE_TYPE_BACKEND)))
    {

        struct vfs_backend *backend = current->data;

        if (backend->id == id)
            return backend;

    }

    return 0;

}

struct vfs_protocol *vfs_findprotocol(struct vfs_backend *backend)
{

    struct resource *current = 0;

    while ((current = resource_findtype(current, RESOURCE_TYPE_PROTOFS)))
    {

        struct vfs_protocol *protocol = current->data;

        if (protocol->match(backend))
            return protocol;

    }

    return 0;

}

void vfs_initbackend(struct vfs_backend *backend, unsigned int id, unsigned int (*read)(struct vfs_backend *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct vfs_backend *self, unsigned int offset, unsigned int count, void *buffer))
{

    resource_init(&backend->resource, RESOURCE_TYPE_BACKEND, backend);

    backend->id = id;
    backend->read = read;
    backend->write = write;

}

void vfs_initprotocol(struct vfs_protocol *protocol, unsigned int (*match)(struct vfs_backend *backend), unsigned int (*root)(struct vfs_backend *backend), unsigned int (*create)(struct vfs_backend *backend, unsigned int id, unsigned int count, const char *name), unsigned int (*destroy)(struct vfs_backend *backend, unsigned int id, unsigned int count, const char *name), unsigned int (*open)(struct vfs_backend *backend, unsigned int id), unsigned int (*close)(struct vfs_backend *backend, unsigned int id), unsigned int (*read)(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*parent)(struct vfs_backend *backend, unsigned int id), unsigned int (*child)(struct vfs_backend *backend, unsigned int id, unsigned int count, const char *path), unsigned long (*getphysical)(struct vfs_backend *backend, unsigned int id))
{

    resource_init(&protocol->resource, RESOURCE_TYPE_PROTOFS, protocol);

    protocol->match = match;
    protocol->root = root;
    protocol->create = create;
    protocol->destroy = destroy;
    protocol->open = open;
    protocol->close = close;
    protocol->read = read;
    protocol->write = write;
    protocol->parent = parent;
    protocol->child = child;
    protocol->getphysical = getphysical;

}

void vfs_initchannel(struct vfs_channel *channel)
{

    resource_init(&channel->resource, RESOURCE_TYPE_CHANNEL, channel);

}

void vfs_initmount(struct vfs_mount *mount)
{

    resource_init(&mount->resource, RESOURCE_TYPE_MOUNT, mount);

}

void vfs_initdescriptor(struct vfs_descriptor *descriptor)
{

    resource_init(&descriptor->resource, RESOURCE_TYPE_DESCRIPTOR, descriptor);

}

