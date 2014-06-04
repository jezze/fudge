#include <kernel.h>
#include "resource.h"
#include "vfs.h"

struct vfs_backend *vfs_find_backend(unsigned int id)
{

    struct resource_item *current = 0;

    while ((current = resource_find_item(current)))
    {

        struct vfs_backend *backend = current->data;

        if (current->type != VFS_RESOURCE_BACKEND)
            continue;

        if (backend->id == id)
            return backend;

    }

    return 0;

}

struct vfs_protocol *vfs_find_protocol(struct vfs_backend *backend)
{

    struct resource_item *current = 0;

    while ((current = resource_find_item(current)))
    {

        struct vfs_protocol *protocol = current->data;

        if (current->type != VFS_RESOURCE_PROTOCOL)
            continue;

        if (protocol->match(backend))
            return protocol;

    }

    return 0;

}

void vfs_init_backend(struct vfs_backend *backend, unsigned int id, unsigned int (*read)(struct vfs_backend *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct vfs_backend *self, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(backend, sizeof (struct vfs_backend));
    resource_init_item(&backend->resource, VFS_RESOURCE_BACKEND, backend);

    backend->id = id;
    backend->read = read;
    backend->write = write;

}

void vfs_init_protocol(struct vfs_protocol *protocol, unsigned int (*match)(struct vfs_backend *backend), unsigned int (*root)(struct vfs_backend *backend), unsigned int (*open)(struct vfs_backend *backend, unsigned int id), unsigned int (*close)(struct vfs_backend *backend, unsigned int id), unsigned int (*read)(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*parent)(struct vfs_backend *backend, unsigned int id), unsigned int (*child)(struct vfs_backend *backend, unsigned int id, unsigned int count, const char *path), unsigned long (*get_physical)(struct vfs_backend *backend, unsigned int id))
{

    memory_clear(protocol, sizeof (struct vfs_protocol));
    resource_init_item(&protocol->resource, VFS_RESOURCE_PROTOCOL, protocol);

    protocol->match = match;
    protocol->root = root;
    protocol->open = open;
    protocol->close = close;
    protocol->read = read;
    protocol->write = write;
    protocol->parent = parent;
    protocol->child = child;
    protocol->get_physical = get_physical;

}

void vfs_init_channel(struct vfs_channel *channel)
{

    memory_clear(channel, sizeof (struct vfs_channel));
    resource_init_item(&channel->resource, VFS_RESOURCE_CHANNEL, channel);

}

void vfs_init_mount(struct vfs_mount *mount)
{

    memory_clear(mount, sizeof (struct vfs_mount));
    resource_init_item(&mount->resource, VFS_RESOURCE_MOUNT, mount);

}

void vfs_init_descriptor(struct vfs_descriptor *descriptor)
{

    memory_clear(descriptor, sizeof (struct vfs_descriptor));
    resource_init_item(&descriptor->resource, VFS_RESOURCE_DESCRIPTOR, descriptor);

}

