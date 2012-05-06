#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/modules.h>
#include <kernel/vfs/root.h>

static union modules_module **modules;
static struct modules_filesystem filesystem;

static unsigned int read(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    char *out = buffer;
    unsigned int length = 0;
    unsigned int i;

    for (i = 0; i < MODULES_MODULE_SLOTS; i++)
    {

        union modules_module *module = modules[i];

        if (!module || module->base.type != MODULES_TYPE_FILESYSTEM)
            continue;

        if (&module->filesystem == self)
            continue;

        unsigned int size = string_length(module->filesystem.path) - 1;

        memory_copy(out + length, module->filesystem.path + 1, size);
        memory_copy(out + length + size, "\n", 1);
        length += size + 1;

    }

    return length;

}

static unsigned int find(struct modules_filesystem *self, char *name)
{

    return 1;

}

void vfs_root_init(union modules_module **m)
{

    modules = m;

    modules_filesystem_init(&filesystem, 0x0001, "root", 0, 0, read, 0, find, 0);
    modules_register_filesystem(&filesystem);
    filesystem.path = "/";

}

