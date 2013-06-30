#include <fudge/kernel.h>
#include "kernel.h"
#include "vfs.h"
#include "vfs_module.h"

static struct module_backend backends[4];

static unsigned int module_read(struct vfs_backend *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct module_backend *backend = (struct module_backend *)self;

    return memory_read(buffer, count, backend->module->base, backend->module->size, offset);

}

static unsigned int module_write(struct vfs_backend *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct module_backend *backend = (struct module_backend *)self;

    return memory_read(backend->module->base, backend->module->size, buffer, count, offset);

}

struct vfs_backend *vfs_module_setup(unsigned int count, struct kernel_module *modules)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        vfs_init_backend(&backends[i].base, module_read, module_write);

        backends[i].module = &modules[i];

    }

    return &backends[0].base;

}

