#include <memory.h>
#include <string.h>
#include <modules.h>
#include <vfs/sys.h>

static unsigned int read_root(struct modules_filesystem *self, unsigned int id, unsigned int count, char *buffer)
{

    memory_copy(buffer, "all/\nbus/\ndevice/\ndriver/\nfilesystem/\nramdisk/\nmodule/\nnet/\ntty/\n", 65);

    return 65;

}

static unsigned int read_category(struct modules_filesystem *self, unsigned int id, unsigned int count, char *buffer)
{

    struct vfs_sys_filesystem *filesystem = (struct vfs_sys_filesystem *)self;
    unsigned int i;
    unsigned int length = 0;

    for (i = 0; i < MODULES_MODULE_SLOTS; i++)
    {

        unsigned int size;
        union modules_module *module = filesystem->modules[i];

        if (!module)
            continue;

        if (id == 2000 && module->base.type != MODULES_TYPE_BUS)
            continue;

        if (id == 3000 && module->base.type != MODULES_TYPE_DEVICE)
            continue;

        if (id == 4000 && module->base.type != MODULES_TYPE_DRIVER)
            continue;

        if (id == 5000 && module->base.type != MODULES_TYPE_FILESYSTEM)
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

    if (offset > 0)
        return 0;

    if (id == 1)
        return read_root(self, id, count, buffer);

    return read_category(self, id, count, buffer);

}

static unsigned int walk_category(struct modules_filesystem *self, unsigned int id, unsigned int count, char *path)
{

    if (!count)
        return id;

    if (memory_match(path, "../", 3))
        return self->walk(self, 1, count - 3, path + 3);

    return 0;

}

static unsigned int walk(struct modules_filesystem *self, unsigned int id, unsigned int count, char *path)
{

    if (!count)
        return id;

    if (memory_match(path, "../", 3))
        return walk(self, id, count - 3, path + 3);

    if (memory_match(path, "all/", 4))
        return walk_category(self, 1000, count - 4, path + 4);

    if (memory_match(path, "bus/", 4))
        return walk_category(self, 2000, count - 4, path + 4);

    if (memory_match(path, "device/", 7))
        return walk_category(self, 3000, count - 7, path + 7);

    if (memory_match(path, "driver/", 7))
        return walk_category(self, 4000, count - 7, path + 7);

    if (memory_match(path, "filesystem/", 11))
        return walk_category(self, 5000, count - 11, path + 11);

    return 0;

}

void vfs_sys_filesystem_init(struct vfs_sys_filesystem *filesystem, union modules_module **modules)
{

    memory_clear(filesystem, sizeof (struct vfs_sys_filesystem));

    modules_filesystem_init(&filesystem->base, 0x0002, 0, 1, "sys", 0, 0, read, 0, walk, 0);

    filesystem->modules = modules;

}

