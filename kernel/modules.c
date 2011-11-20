#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <modules/stream/stream.h>

static struct vfs_view modulesViewDev;
static struct modules_bus *modulesBusses[MODULES_BUS_SLOTS];
static struct modules_device *modulesDevices[MODULES_DEVICE_SLOTS];
static struct modules_driver *modulesDrivers[MODULES_DRIVER_SLOTS];
static struct vfs_filesystem modulesFilesystem;

struct modules_bus *modules_get_bus(unsigned int type)
{

    unsigned int i;

    for (i = 0; i < MODULES_BUS_SLOTS; i++)
    {

        if (!modulesBusses[i])
            continue;

        if (modulesBusses[i]->type == type)
            return modulesBusses[i];

    }

    return 0;

}

struct modules_device *modules_get_device(unsigned int type)
{

    unsigned int i;

    for (i = 0; i < MODULES_DEVICE_SLOTS; i++)
    {

        if (!modulesDevices[i])
            continue;

        if (modulesDevices[i]->type == type)
            return modulesDevices[i];

    }

    return 0;

}

struct modules_driver *modules_get_driver(unsigned int type)
{

    unsigned int i;

    for (i = 0; i < MODULES_DRIVER_SLOTS; i++)
    {

        if (!modulesDrivers[i])
            continue;

        if (modulesDrivers[i]->type == type)
            return modulesDrivers[i];

    }

    return 0;

}

void modules_register_bus(struct modules_bus *bus)
{

    unsigned int i;

    for (i = 0; i < MODULES_BUS_SLOTS; i++)
    {

        if (modulesBusses[i])
            continue;

        modulesBusses[i] = bus;

        break;

    }

}

void modules_register_device(struct modules_device *device)
{

    unsigned int i;

    for (i = 0; i < MODULES_DEVICE_SLOTS; i++)
    {

        if (modulesDevices[i])
            continue;

        modulesDevices[i] = device;

        break;

    }

}

void modules_register_driver(struct modules_driver *driver)
{

    unsigned int i;

    for (i = 0; i < MODULES_DRIVER_SLOTS; i++)
    {

        if (modulesDrivers[i])
            continue;

        modulesDrivers[i] = driver;

        break;

    }

}

static struct vfs_node *modules_filesystem_view_find_node(struct vfs_view *self, char *name)
{

    unsigned int i;

    for (i = 0; modulesDevices[i]; i++)
    {

        if (modulesDevices[i]->type != STREAM_DEVICE_TYPE)
            continue;

        unsigned int count = string_length(((struct stream_device *)modulesDevices[i])->name) + 1;

        if (!memory_compare(name, ((struct stream_device *)modulesDevices[i])->name, count))
            return &((struct stream_device *)modulesDevices[i])->node;

    }

    return 0;

}

static unsigned int modules_filesystem_view_read(struct vfs_view *self, unsigned int count, void *buffer)
{

    memory_set(buffer, 0, 1);
    unsigned int i;

    for (i = 0; modulesDevices[i]; i++)
    {

        if (modulesDevices[i]->type != STREAM_DEVICE_TYPE)
            continue;

        string_write_concat(buffer, ((struct stream_device *)modulesDevices[i])->name);
        string_write_concat(buffer, "\n");

    }

    return string_length(buffer);

}

static struct vfs_view *modules_filesystem_find_view(struct vfs_filesystem *self, char *name)
{

    return &modulesViewDev;

}

void modules_module_init(struct modules_module *module, unsigned int type)
{

    module->type = type;

}

void modules_bus_init(struct modules_bus *bus, unsigned int type)
{

    bus->type = type;
    bus->module.type = MODULES_TYPE_BUS;

}

void modules_device_init(struct modules_device *device, unsigned int type)
{

    device->type = type;
    device->module.type = MODULES_TYPE_DEVICE;

}

void modules_driver_init(struct modules_driver *driver, unsigned int type)
{

    driver->type = type;
    driver->module.type = MODULES_TYPE_DRIVER;

}

void modules_init()
{

    vfs_view_init(&modulesViewDev, "dev", modules_filesystem_view_find_node, modules_filesystem_view_read);
    vfs_filesystem_init(&modulesFilesystem, modules_filesystem_find_view); 
    vfs_register_filesystem(&modulesFilesystem);

}

