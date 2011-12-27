#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>

static struct modules_module *modules[MODULES_MODULE_SLOTS];
static struct vfs_filesystem filesystem;

void modules_foreach(unsigned int (*test)(struct modules_module *module), void (*callback)(struct modules_module *module))
{

    unsigned int i;

    for (i = 0; i < MODULES_MODULE_SLOTS; i++)
    {

        struct modules_module *module = modules[i];

        if (!module)
            continue;

        if (test(module))
            callback(module);

    }

}

unsigned int modules_is_bus(struct modules_module *module)
{

    return module->type == MODULES_TYPE_BUS;

}

unsigned int modules_is_device(struct modules_module *module)
{

    return module->type == MODULES_TYPE_DEVICE;

}

unsigned int modules_is_driver(struct modules_module *module)
{

    return module->type == MODULES_TYPE_DRIVER;

}

struct modules_bus *modules_get_bus(unsigned int type, unsigned int index)
{

    unsigned int i;

    for (i = 0; i < MODULES_MODULE_SLOTS; i++)
    {

        if (modules[i]->type != MODULES_TYPE_BUS)
            continue;

        if (index)
        {

            index--;

            continue;

        }

        struct modules_bus *bus = (struct modules_bus *)modules[i];

        if (bus->type == type)
            return bus;

    }

    return 0;

}

struct modules_device *modules_get_device(unsigned int type, unsigned int index)
{

    unsigned int i;

    for (i = 0; i < MODULES_MODULE_SLOTS; i++)
    {

        if (modules[i]->type != MODULES_TYPE_DEVICE)
            continue;

        if (index)
        {

            index--;

            continue;

        }

        struct modules_device *device = (struct modules_device *)modules[i];

        if (device->type == type)
            return device;

    }

    return 0;

}

struct modules_driver *modules_get_driver(unsigned int type, unsigned int index)
{

    unsigned int i;

    for (i = 0; i < MODULES_MODULE_SLOTS; i++)
    {

        if (modules[i]->type != MODULES_TYPE_DRIVER)
            continue;

        if (index)
        {

            index--;

            continue;

        }

        struct modules_driver *driver = (struct modules_driver *)modules[i];

        if (driver->type == type)
            return driver;

    }

    return 0;

}

void modules_register_module(struct modules_module *module)
{

    unsigned int i;

    for (i = 0; i < MODULES_MODULE_SLOTS; i++)
    {

        if (modules[i])
            continue;

        modules[i] = module;

        break;

    }

}

void modules_register_bus(struct modules_bus *bus)
{

    modules_register_module(&bus->module);

}

void modules_register_device(struct modules_device *device)
{

    modules_register_module(&device->module);

}

void modules_register_driver(struct modules_driver *driver)
{

    modules_register_module(&driver->module);

}

void modules_unregister_module(struct modules_module *module)
{

    unsigned int i;

    for (i = 0; i < MODULES_MODULE_SLOTS; i++)
    {

        if (!modules[i])
            continue;

        if (modules[i] != module)
            continue;

        modules[i] = 0;

        break;

    }

}

void modules_unregister_bus(struct modules_bus *bus)
{

    modules_unregister_module(&bus->module);

}

void modules_unregister_device(struct modules_device *device)
{

    modules_unregister_module(&device->module);

}

void modules_unregister_driver(struct modules_driver *driver)
{

    modules_unregister_module(&driver->module);

}

static struct vfs_view *modules_filesystem_find_view(struct vfs_filesystem *self, char *name)
{

    unsigned int i;

    for (i = 0; i < MODULES_MODULE_SLOTS; i++)
    {

        if (!modules[i])
            continue;

        if (!modules[i]->view)
            continue;

        if (!string_compare(modules[i]->view->name, name))
            return modules[i]->view;

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

