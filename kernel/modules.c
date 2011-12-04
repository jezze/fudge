#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>

static struct modules_bus *busses[MODULES_BUS_SLOTS];
static struct modules_device *devices[MODULES_DEVICE_SLOTS];
static struct modules_driver *drivers[MODULES_DRIVER_SLOTS];
static struct vfs_filesystem filesystem;

struct modules_bus *modules_get_bus(unsigned int type)
{

    unsigned int i;

    for (i = 0; i < MODULES_BUS_SLOTS; i++)
    {

        if (!busses[i])
            continue;

        if (busses[i]->type == type)
            return busses[i];

    }

    return 0;

}

struct modules_device *modules_get_device(unsigned int type)
{

    unsigned int i;

    for (i = 0; i < MODULES_DEVICE_SLOTS; i++)
    {

        if (!devices[i])
            continue;

        if (devices[i]->type == type)
            return devices[i];

    }

    return 0;

}

struct modules_driver *modules_get_driver(unsigned int type)
{

    unsigned int i;

    for (i = 0; i < MODULES_DRIVER_SLOTS; i++)
    {

        if (!drivers[i])
            continue;

        if (drivers[i]->type == type)
            return drivers[i];

    }

    return 0;

}

void modules_register_bus(struct modules_bus *bus)
{

    unsigned int i;

    for (i = 0; i < MODULES_BUS_SLOTS; i++)
    {

        if (busses[i])
            continue;

        busses[i] = bus;

        break;

    }

}

void modules_register_device(struct modules_device *device)
{

    unsigned int i;

    for (i = 0; i < MODULES_DEVICE_SLOTS; i++)
    {

        if (devices[i])
            continue;

        devices[i] = device;

        break;

    }

}

void modules_register_driver(struct modules_driver *driver)
{

    unsigned int i;

    for (i = 0; i < MODULES_DRIVER_SLOTS; i++)
    {

        if (drivers[i])
            continue;

        drivers[i] = driver;

        break;

    }

}

void modules_unregister_bus(struct modules_bus *bus)
{

    unsigned int i;

    for (i = 0; i < MODULES_BUS_SLOTS; i++)
    {

        if (!busses[i])
            continue;

        if (busses[i] != bus)
            continue;

        busses[i] = 0;

        break;

    }

}

void modules_unregister_device(struct modules_device *device)
{

    unsigned int i;

    for (i = 0; i < MODULES_DEVICE_SLOTS; i++)
    {

        if (!devices[i])
            continue;

        if (devices[i] != device)
            continue;

        devices[i] = 0;

        break;

    }

}

void modules_unregister_driver(struct modules_driver *driver)
{

    unsigned int i;

    for (i = 0; i < MODULES_DRIVER_SLOTS; i++)
    {

        if (!drivers[i])
            continue;

        if (drivers[i] != driver)
            continue;

        drivers[i] = 0;

        break;

    }

}

static struct vfs_view *modules_filesystem_find_view(struct vfs_filesystem *self, char *name)
{

    unsigned int i;

    for (i = 0; i < MODULES_DEVICE_SLOTS; i++)
    {

        if (!devices[i])
            continue;

        struct modules_module *module = &devices[i]->module;

        if (!module->view)
            continue;

        if (!string_compare(module->view->name, name))
            return module->view;

    }

    return 0;

}

void modules_module_init(struct modules_module *module, unsigned int type)
{

    module->type = type;
    module->view = 0;

}

void modules_bus_init(struct modules_bus *bus, unsigned int type)
{

    bus->type = type;

    modules_module_init(&bus->module, MODULES_TYPE_BUS);

}

void modules_device_init(struct modules_device *device, unsigned int type)
{

    device->type = type;

    modules_module_init(&device->module, MODULES_TYPE_DEVICE);

}

void modules_driver_init(struct modules_driver *driver, unsigned int type)
{

    driver->type = type;

    modules_module_init(&driver->module, MODULES_TYPE_DRIVER);

}

void modules_init()
{

    vfs_filesystem_init(&filesystem, modules_filesystem_find_view); 
    vfs_register_filesystem(&filesystem);

}

