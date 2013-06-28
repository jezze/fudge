#include <fudge/kernel.h>
#include "vfs.h"
#include "vfs_module.h"

static struct module_backend backends[4];

static unsigned int module_read(struct vfs_backend *self, unsigned int offset, unsigned int count, void *buffer)
{

    return memory_read(buffer, count, "TAR", 3, offset);

}

static unsigned int module_write(struct vfs_backend *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

struct vfs_backend *vfs_module_setup(unsigned int modulesc, void **modulesv)
{

    unsigned int i;

    for (i = 0; i < modulesc; i++)
    {

        vfs_init_backend(&backends[i].base, module_read, module_write);

        backends[i].address = modulesv[i];
        backends[i].size = 0xFFFFFFFF;

    }

    return &backends[0].base;

}

