#include <kernel.h>
#include "resource.h"
#include "vfs.h"
#include "task.h"
#include "container.h"
#include "kernel.h"

static struct list *ref;

static unsigned int backend_read(struct vfs_backend *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int backend_write(struct vfs_backend *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int root(struct vfs_backend *backend)
{

    return RESOURCE_TYPE_ALL;

}

static unsigned int parent(struct vfs_backend *backend, unsigned int id)
{

    return RESOURCE_TYPE_ALL;

}

static unsigned int match(struct vfs_backend *backend)
{

    return 0;

}

static unsigned long get_physical(struct vfs_backend *backend, unsigned int id)
{

    return 0;

}

static unsigned int open(struct vfs_backend *backend, unsigned int id)
{

    return id;

}

static unsigned int close(struct vfs_backend *backend, unsigned int id)
{

    return 0;

}

static unsigned int read(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    if (id == RESOURCE_TYPE_ALL)
    {

        char temp[4096];
        struct list_item *current;
        unsigned int o = 0;

        o = memory_write(temp, 4096, "../\n", 4, o);

        for (current = ref->head; current; current = current->next)
        {

            struct resource_list *list = current->data;

            o += memory_write(temp, 4096, list->name, ascii_length(list->name), o);
            o += memory_write(temp, 4096, "/\n", 2, o);

        }

        return memory_read(buffer, count, temp, o, offset);

    }

    if (id > RESOURCE_TYPE_ALL)
    {

        return memory_read(buffer, count, "../\n0/\n1/\n2/\n", 13, offset);

    }

    return 0;

}

static unsigned int write(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int walk(struct vfs_backend *backend, unsigned int id, unsigned int count, const char *path)
{

    unsigned int n = vfs_findnext(count, path);

    if (!count)
        return id;

    if (id == RESOURCE_TYPE_ALL)
    {

        struct list_item *current;

        for (current = ref->head; current; current = current->next)
        {

            struct resource_list *list = current->data;
            unsigned int l = ascii_length(list->name);

            if (memory_match(path, list->name, l))
                return walk(backend, list->type, count - n, path + n);

        }

    }

    return 0;

}

void vfs_init_kernel(struct list *resources, struct vfs_backend *backend, struct vfs_protocol *protocol)
{

    ref = resources;

    memory_clear(backend, sizeof (struct vfs_backend));
    vfs_init_backend(backend, backend_read, backend_write);

    memory_clear(protocol, sizeof (struct vfs_protocol));
    vfs_init_protocol(protocol, match, root, open, close, read, write, parent, walk, get_physical);

}

