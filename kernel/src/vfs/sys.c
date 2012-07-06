#include <memory.h>
#include <string.h>
#include <modules.h>
#include <vfs/sys.h>

static union modules_module **modules;
static struct modules_filesystem filesystem;

static unsigned int read(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int i;
    unsigned int length = 0;
    char *out = buffer;

    if (id == 1)
    {

        memory_copy(buffer, "all/\nbus/\ndevice/\ndriver/\n", 26);

        return 26;

    }

    for (i = 0; i < MODULES_MODULE_SLOTS; i++)
    {

        unsigned int size;
        union modules_module *module = modules[i];

        if (!module)
            continue;

        if (id != 2)
        {

            if (id == 3 && module->base.type != MODULES_TYPE_BUS)
                continue;

            if (id == 4 && module->base.type != MODULES_TYPE_DEVICE)
                continue;

            if (id == 5 && module->base.type != MODULES_TYPE_DRIVER)
                continue;

        }

        size = string_length(module->base.name);

        memory_copy(out + length, module->base.name, size);
        memory_copy(out + length + size, "\n", 1);
        length += size + 1;

    }

    return length;

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

    return 0;

}

void vfs_sys_setup(union modules_module **m)
{

    modules = m;

    modules_filesystem_init(&filesystem, 0x0001, "sys", 0, 0, read, 0, walk, 0);
    modules_register_filesystem(&filesystem);
    filesystem.path = "/sys/";

}

