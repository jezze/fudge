#include <memory.h>
#include <string.h>
#include <modules.h>
#include <vfs/sys.h>

static union modules_module **modules;
static struct modules_filesystem filesystem;

static unsigned int read_root(unsigned int count, void *buffer)
{

    memory_copy(buffer, "all/\nbus/\ndevice/\ndriver/\nfilesystem/\nramdisk/\nmodule/\n", 55);

    return 55;

}

static unsigned int read_category(unsigned int id, unsigned int count, char *buffer)
{

    unsigned int i;
    unsigned int length = 0;

    for (i = 0; i < MODULES_MODULE_SLOTS; i++)
    {

        unsigned int size;
        union modules_module *module = modules[i];

        if (!module)
            continue;

        if (id == 3 && module->base.type != MODULES_TYPE_BUS)
            continue;

        if (id == 4 && module->base.type != MODULES_TYPE_DEVICE)
            continue;

        if (id == 5 && module->base.type != MODULES_TYPE_DRIVER)
            continue;

        if (id == 6 && module->base.type != MODULES_TYPE_FILESYSTEM)
            continue;

        size = string_length(module->base.name);

        memory_copy(buffer + length, module->base.name, size);
        memory_copy(buffer + length + size, "/\n", 2);
        length += size + 2;

    }

    return length;

}

static unsigned int read(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    if (id == 1)
        return read_root(count, buffer);

    return read_category(id, count, buffer);

}

static unsigned int walk(struct modules_filesystem *self, unsigned int id, unsigned int count, void *buffer)
{

    if (!count)
        return 1;

    if (memory_compare(buffer, "all/", 4))
        return 2;

    if (memory_compare(buffer, "bus/", 4))
        return 3;

    if (memory_compare(buffer, "device/", 7))
        return 4;

    if (memory_compare(buffer, "driver/", 7))
        return 5;

    if (memory_compare(buffer, "filesystem/", 7))
        return 6;

    return 0;

}

static unsigned int parent(struct modules_filesystem *self, unsigned int id)
{

    return 1;

}

struct modules_filesystem *vfs_sys_setup(union modules_module **m)
{

    modules = m;

    modules_filesystem_init(&filesystem, 0x0001, "sys", 0, 0, read, 0, parent, walk, 0);
    modules_register_filesystem(&filesystem);

    return &filesystem;

}

