#include <memory.h>
#include <string.h>
#include <modules.h>
#include <vfs/sys.h>

static struct vfs_sys_filesystem filesystem;

static unsigned int parent(struct modules_filesystem *self, unsigned int id)
{

    return 1;

}

static unsigned int read_root(unsigned int count, void *buffer)
{

    memory_copy(buffer, "all/\nbus/\ndevice/\ndriver/\nfilesystem/\nramdisk/\nmodule/\n", 55);

    return 55;

}

static unsigned int read_category(struct vfs_sys_filesystem *filesystem, unsigned int id, unsigned int count, char *buffer)
{

    unsigned int i;
    unsigned int length = 0;

    for (i = 0; i < MODULES_MODULE_SLOTS; i++)
    {

        unsigned int size;
        union modules_module *module = filesystem->modules[i];

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

    struct vfs_sys_filesystem *filesystem = (struct vfs_sys_filesystem *)self;

    if (offset > 0)
        return 0;

    if (id == 1)
        return read_root(count, buffer);

    return read_category(filesystem, id, count, buffer);

}

static unsigned int walk(struct modules_filesystem *self, unsigned int id, unsigned int count, void *buffer)
{

    if (!count)
        return 1;

    if (memory_match(buffer, "all/", 4))
        return 2;

    if (memory_match(buffer, "bus/", 4))
        return 3;

    if (memory_match(buffer, "device/", 7))
        return 4;

    if (memory_match(buffer, "driver/", 7))
        return 5;

    if (memory_match(buffer, "filesystem/", 7))
        return 6;

    return 0;

}

void vfs_sys_filesystem_init(struct vfs_sys_filesystem *filesystem, union modules_module **modules)
{

    memory_clear(filesystem, sizeof (struct vfs_sys_filesystem));

    modules_filesystem_init(&filesystem->base, 0x0002, "sys", 0, 0, read, 0, parent, walk, 0);

    filesystem->modules = modules;

}

struct modules_filesystem *vfs_sys_setup(union modules_module **modules)
{

    vfs_sys_filesystem_init(&filesystem, modules);
    modules_register_filesystem(&filesystem.base);

    return &filesystem.base;

}

