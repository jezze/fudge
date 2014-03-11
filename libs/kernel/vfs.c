#include <kernel.h>
#include "resource.h"
#include "vfs.h"

static struct resource_iterator backends;
static struct resource_iterator protocols;

static unsigned int backends_match(struct resource_item *item)
{

    return item->type == RESOURCE_TYPE_VFSBACKEND;

}

static unsigned int protocols_match(struct resource_item *item)
{

    return item->type == RESOURCE_TYPE_VFSPROTOCOL;

}

unsigned int vfs_findnext(unsigned int count, const char *path)
{

    unsigned int offset = memory_findbyte(path, count, '/');

    return offset == count ? offset : offset + 1;

}

unsigned int vfs_isparent(unsigned int count, const char *path)
{

    return (count >= 3) ? memory_match(path, "../", 3) : 0;

}

struct vfs_backend *vfs_find_backend()
{

    struct resource_item *current = 0;
    struct resource_item *last = 0;
    struct vfs_backend *backend = 0;

    while ((current = resource_find_item(&backends, current)))
        last = current;

    backend = last->data;

    return backend;

}

struct vfs_protocol *vfs_find_protocol(struct vfs_backend *backend)
{

    struct resource_item *current = 0;

    while ((current = resource_find_item(&protocols, current)))
    {

        struct vfs_protocol *protocol = current->data;

        if (protocol->match(backend))
            return protocol;

    }

    return 0;

}

void vfs_init_backend(struct vfs_backend *backend, unsigned int (*read)(struct vfs_backend *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct vfs_backend *self, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(backend, sizeof (struct vfs_backend));
    resource_init_item(&backend->resource, RESOURCE_TYPE_VFSBACKEND, backend);

    backend->read = read;
    backend->write = write;

}

void vfs_init_protocol(struct vfs_protocol *protocol, unsigned int (*match)(struct vfs_backend *backend), unsigned int (*root)(struct vfs_backend *backend), unsigned int (*open)(struct vfs_backend *backend, unsigned int id), unsigned int (*close)(struct vfs_backend *backend, unsigned int id), unsigned int (*read)(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*parent)(struct vfs_backend *backend, unsigned int id), unsigned int (*walk)(struct vfs_backend *backend, unsigned int id, unsigned int count, const char *path), unsigned long (*get_physical)(struct vfs_backend *backend, unsigned int id))
{

    memory_clear(protocol, sizeof (struct vfs_protocol));
    resource_init_item(&protocol->resource, RESOURCE_TYPE_VFSPROTOCOL, protocol);

    protocol->match = match;
    protocol->root = root;
    protocol->open = open;
    protocol->close = close;
    protocol->read = read;
    protocol->write = write;
    protocol->parent = parent;
    protocol->walk = walk;
    protocol->get_physical = get_physical;

}

void vfs_init_channel(struct vfs_channel *channel, struct vfs_backend *backend, struct vfs_protocol *protocol)
{

    memory_clear(channel, sizeof (struct vfs_channel));
    resource_init_item(&channel->resource, RESOURCE_TYPE_VFSCHANNEL, channel);

    channel->backend = backend;
    channel->protocol = protocol;

}

void vfs_init_descriptor(struct vfs_descriptor *descriptor, struct vfs_channel *channel, unsigned int id)
{

    memory_clear(descriptor, sizeof (struct vfs_descriptor));
    resource_init_item(&descriptor->resource, RESOURCE_TYPE_VFSDESCRIPTOR, descriptor);

    descriptor->channel = channel;
    descriptor->id = id;

}

void vfs_setup()
{

    resource_init_iterator(&backends, backends_match);
    resource_init_iterator(&protocols, protocols_match);

}

