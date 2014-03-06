#include <kernel.h>
#include "resource.h"
#include "vfs.h"

static struct list alllist;
static struct list backendlist;
static struct list protocollist;
static struct resource_item all;
static struct resource_item backends;
static struct resource_item protocols;

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

    return backendlist.tail->data;

}

struct vfs_protocol *vfs_find_protocol(struct vfs_backend *backend)
{

    struct list_item *current;

    for (current = protocollist.head; current; current = current->next)
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
    resource_init_item(&backend->resource, backend, RESOURCE_TYPE_VFSBACKEND, 7, "backend");

    backend->read = read;
    backend->write = write;

}

void vfs_init_protocol(struct vfs_protocol *protocol, unsigned int (*match)(struct vfs_backend *backend), unsigned int (*root)(struct vfs_backend *backend), unsigned int (*open)(struct vfs_backend *backend, unsigned int id), unsigned int (*close)(struct vfs_backend *backend, unsigned int id), unsigned int (*read)(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*parent)(struct vfs_backend *backend, unsigned int id), unsigned int (*walk)(struct vfs_backend *backend, unsigned int id, unsigned int count, const char *path), unsigned long (*get_physical)(struct vfs_backend *backend, unsigned int id))
{

    memory_clear(protocol, sizeof (struct vfs_protocol));
    resource_init_item(&protocol->resource, protocol, RESOURCE_TYPE_VFSPROTOCOL, 8, "protocol");

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

void vfs_setup()
{

    list_init(&alllist);
    list_init(&backendlist);
    list_init(&protocollist);
    resource_init_item(&all, &alllist, RESOURCE_TYPE_VFS, 3, "vfs");
    resource_register_item(&all);
    resource_init_item(&backends, &backendlist, RESOURCE_TYPE_VFSBACKEND, 8, "backends");
    resource_register_item(&backends);
    resource_init_item(&protocols, &protocollist, RESOURCE_TYPE_VFSPROTOCOL, 9, "protocols");
    resource_register_item(&protocols);

}

void vfs_register_backend(struct resource_item *item)
{

    list_add(&backendlist, &item->item);

}

void vfs_register_protocol(struct resource_item *item)
{

    list_add(&protocollist, &item->item);

}

