#include <memory.h>
#include <string.h>
#include <modules.h>
#include <vfs/root.h>

static union modules_module **modules;
static struct modules_filesystem filesystem;

static unsigned int read(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int i;
    unsigned int length = 0;
    char *out = buffer;

    for (i = 0; i < MODULES_MODULE_SLOTS; i++)
    {

        unsigned int size;
        union modules_module *module = modules[i];

        if (!module || module->base.type != MODULES_TYPE_FILESYSTEM)
            continue;

        if (&module->filesystem == self)
            continue;

        size = string_length(module->filesystem.path) - 1;

        memory_copy(out + length, module->filesystem.path + 1, size);
        memory_copy(out + length + size, "\n", 1);
        length += size + 1;

    }

    return length;

}

static unsigned int walk(struct modules_filesystem *self, unsigned int id, char *name)
{

    return 1;

}

void vfs_root_init(union modules_module **m)
{

    modules = m;

    modules_filesystem_init(&filesystem, 0x0001, "root", 0, 0, read, 0, walk, 0);
    modules_register_filesystem(&filesystem);
    filesystem.path = "/";

}

