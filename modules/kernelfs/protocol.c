#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include "protocol.h"

#define KERNELFS_STATIC                 0x01
#define KERNELFS_DYNAMIC                0x02

struct kernelfs_ops
{

    void (*read)(struct kernelfs_ops *self, unsigned int index);

};

struct kernelfs_node
{

    struct kernelfs_node *child;
    char *path;
    unsigned char options;

};

static struct kernelfs_node id[] = {
    {0, 0, 0}
};

static struct kernelfs_node memory[] = {
    {0, 0, 0}
};

static struct kernelfs_node task[] = {
    {id, "id/", KERNELFS_STATIC},
    {memory, "memory/", KERNELFS_STATIC},
    {0, 0, 0}
};

static struct kernelfs_node tasks[] = {
    {task, 0, KERNELFS_DYNAMIC},
    {0, 0, 0}
};

static struct kernelfs_node modules[] = {
    {task, 0, KERNELFS_DYNAMIC},
    {0, 0, 0}
};

static struct kernelfs_node kroot[] = {
    {tasks, "task/", KERNELFS_STATIC},
    {modules, "module/", KERNELFS_STATIC},
    {0, 0, 0}
};

static unsigned int root(struct vfs_backend *backend)
{

    return (unsigned int)kroot;

}

static unsigned int parent(struct vfs_backend *backend, unsigned int id)
{

    return 0;

}

static unsigned int child(struct vfs_backend *backend, unsigned int id, unsigned int count, const char *path)
{

    return 0;

}

static unsigned int match(struct vfs_backend *backend)
{

    unsigned char buffer[8];

    backend->read(backend, 0, 8, buffer);

    return memory_match(buffer, "KERNELFS", 8);

}

static unsigned int create(struct vfs_backend *backend, unsigned int id, unsigned int count, const char *name)
{

    return 0;

}

static unsigned int destroy(struct vfs_backend *backend, unsigned int id, unsigned int count, const char *name)
{

    return 0;

}

static unsigned int open(struct vfs_backend *backend, unsigned int id)
{

    return id;

}

static unsigned int close(struct vfs_backend *backend, unsigned int id)
{

    return id;

}

static unsigned int read(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct kernelfs_node *node = (struct kernelfs_node *)id;
    unsigned int i;

    if (!node->child)
        return 0; /* return read_ops */

    for (i = 0; node[i].child; i++)
    {

    }

    return 0;

}

static unsigned int write(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

void kernelfs_init_protocol(struct vfs_protocol *protocol)
{

    memory_clear(protocol, sizeof (struct vfs_protocol));
    vfs_init_protocol(protocol, match, root, create, destroy, open, close, read, write, parent, child, 0);

}

