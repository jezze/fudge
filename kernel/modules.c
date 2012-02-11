#include <lib/memory.h>
#include <kernel/modules.h>

static struct modules_module *modules[MODULES_MODULE_SLOTS];

static void modules_attach(struct modules_driver *driver)
{

    if (!driver->check)
        return;

    unsigned int i;

    for (i = 0; i < MODULES_MODULE_SLOTS; i++)
    {

        struct modules_module *module = modules[i];

        if (!module || module->type != MODULES_TYPE_DEVICE)
            continue;

        struct modules_device *device = (struct modules_device *)module;

        if (device->driver)
            continue;

        if (!driver->check(driver, device))
            continue;

        device->driver = driver;

        if (driver->attach)
            driver->attach(driver, device);

    }

}

struct modules_bus *modules_get_bus(unsigned int type)
{

    unsigned int i;

    for (i = 0; i < MODULES_MODULE_SLOTS; i++)
    {

        struct modules_module *module = modules[i];

        if (!module || module->type != MODULES_TYPE_BUS)
            continue;

        struct modules_bus *bus = (struct modules_bus *)module;

        if (bus->type == type)
            return bus;

    }

    return 0;

}

struct modules_device *modules_get_device(unsigned int type)
{

    unsigned int i;

    for (i = 0; i < MODULES_MODULE_SLOTS; i++)
    {

        struct modules_module *module = modules[i];

        if (!module || module->type != MODULES_TYPE_DEVICE)
            continue;

        struct modules_device *device = (struct modules_device *)module;

        if (device->type == type)
            return device;

    }

    return 0;

}

struct modules_driver *modules_get_driver(unsigned int type)
{

    unsigned int i;

    for (i = 0; i < MODULES_MODULE_SLOTS; i++)
    {

        struct modules_module *module = modules[i];

        if (!module || module->type != MODULES_TYPE_DRIVER)
            continue;

        struct modules_driver *driver = (struct modules_driver *)module;

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
    modules_attach(driver);

    if (driver->start)
        driver->start(driver);

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

void modules_module_init(struct modules_module *module, unsigned int type)
{

    memory_clear(module, sizeof (struct modules_module));

    module->type = type;

}

void modules_bus_init(struct modules_bus *bus, unsigned int type)
{

    memory_clear(bus, sizeof (struct modules_bus));

    modules_module_init(&bus->module, MODULES_TYPE_BUS);

    bus->type = type;

}

void modules_device_init(struct modules_device *device, unsigned int type)
{

    memory_clear(device, sizeof (struct modules_device));

    modules_module_init(&device->module, MODULES_TYPE_DEVICE);

    device->type = type;

}

void modules_driver_init(struct modules_driver *driver, unsigned int type)
{

    memory_clear(driver, sizeof (struct modules_driver));

    modules_module_init(&driver->module, MODULES_TYPE_DRIVER);

    driver->type = type;

}

