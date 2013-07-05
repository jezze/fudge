#include <fudge/kernel.h>
#include "vfs.h"

static struct vfs_protocol *protocols;

unsigned int vfs_findnext(unsigned int count, const char *path)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        if (path[i] == '/')
            return i + 1;

    }

    return count;

}

unsigned int vfs_isparent(unsigned int count, const char *path)
{

    return (count >= 3) ? memory_match(path, "../", 3) : 0;

}

struct vfs_protocol *vfs_get_protocol(struct vfs_backend *backend)
{

    struct vfs_protocol *current;

    for (current = protocols; current; current = current->sibling)
    {

        if (current->match(backend))
            return current;

    }

    return 0;

}

void vfs_register_protocol(struct vfs_protocol *protocol)
{

    struct vfs_protocol *current;

    if (!protocols)
    {

        protocols = protocol;

        return;

    }

    for (current = protocols; current; current = current->sibling)
    {

        if (current->sibling)
            continue;

        current->sibling = protocol;

        return;

    }

}

void vfs_unregister_protocol(struct vfs_protocol *protocol)
{

    struct vfs_protocol *current;

    if (protocols == protocol)
    {

        protocols = protocols->sibling;

        return;

    }

    for (current = protocols; current; current = current->sibling)
    {

        if (current->sibling != protocol)
            continue;

        current->sibling = current->sibling->sibling;

        return;

    }

}

void vfs_init_backend(struct vfs_backend *backend, unsigned int (*read)(struct vfs_backend *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct vfs_backend *self, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(backend, sizeof (struct vfs_backend));

    backend->read = read;
    backend->write = write;

}

void vfs_init_protocol(struct vfs_protocol *protocol, unsigned int rootid, unsigned int (*match)(struct vfs_backend *backend), unsigned int (*open)(struct vfs_backend *backend, unsigned int id), unsigned int (*close)(struct vfs_backend *backend, unsigned int id), unsigned int (*read)(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*walk)(struct vfs_backend *backend, unsigned int id, unsigned int count, const char *path), unsigned int (*get_physical)(struct vfs_backend *backend, unsigned int id))
{

    memory_clear(protocol, sizeof (struct vfs_protocol));

    protocol->rootid = rootid;
    protocol->match = match;
    protocol->open = open;
    protocol->close = close;
    protocol->read = read;
    protocol->write = write;
    protocol->walk = walk;
    protocol->get_physical = get_physical;

}

void vfs_setup()
{

    protocols = 0;

}

