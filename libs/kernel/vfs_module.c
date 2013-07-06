#include <fudge/kernel.h>
#include "kernel.h"
#include "vfs.h"
#include "vfs_module.h"

static struct backend
{

    struct vfs_backend base;
    struct kernel_module *module;

} backends[4];

static unsigned int read(struct vfs_backend *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct backend *b = (struct backend *)self;

    return memory_read(buffer, count, b->module->base, b->module->size, offset);

}

static unsigned int write(struct vfs_backend *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct backend *b = (struct backend *)self;

    return memory_write(b->module->base, b->module->size, buffer, count, offset);

}

struct vfs_backend *vfs_module_setup(unsigned int count, struct kernel_module *modules)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        vfs_init_backend(&backends[i].base, read, write);

        backends[i].module = &modules[i];

    }

    return &backends[0].base;

}

