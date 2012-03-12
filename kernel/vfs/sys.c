#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/modules.h>
#include <kernel/vfs.h>
#include <kernel/vfs/sys.h>

static struct modules_module **modules;
static struct vfs_filesystem filesystem;

static unsigned int read(struct vfs_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    if (id == 1)
    {

        memory_copy(buffer, "./\n../\nall/\nbus/\ndevice/\ndriver/\n", 33);

        return 33;

    }

    string_write(buffer, "./\n../\n", 7);

    unsigned int length = 7;
    unsigned int i;

    for (i = 0; i < MODULES_MODULE_SLOTS; i++)
    {

        if (!modules[i])
            continue;

        if (id != 2)
        {

            if (id == 3 && modules[i]->type != MODULES_TYPE_BUS)
                continue;

            if (id == 4 && modules[i]->type != MODULES_TYPE_DEVICE)
                continue;

            if (id == 5 && modules[i]->type != MODULES_TYPE_DRIVER)
                continue;

        }

        string_write(buffer + length, "%s\n", modules[i]->name);
        length += string_length(buffer + length);

    }

    return length;

}

static unsigned int find(struct vfs_filesystem *self, char *name)
{

    unsigned int length = string_length(name);

    if (!length)
        return 1;

    if (!memory_compare(name, "all/", 4))
        return 2;

    if (!memory_compare(name, "bus/", 4))
        return 3;

    if (!memory_compare(name, "device/", 7))
        return 4;

    if (!memory_compare(name, "driver/", 7))
        return 5;

    return 0;

}

void vfs_sys_init(struct modules_module **m)
{

    modules = m;

    vfs_filesystem_init(&filesystem, 0, 0, read, 0, find, 0);
    vfs_mount(&filesystem, "/sys/");

}

