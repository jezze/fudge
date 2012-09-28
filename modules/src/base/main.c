#include <memory.h>
#include <base/base.h>

static struct base_module *modules[BASE_MODULE_SLOTS];

static unsigned int attach(struct base_driver *driver)
{

    unsigned int i;
    unsigned int found = 0;

    for (i = 0; i < BASE_MODULE_SLOTS; i++)
    {

        struct base_device *device = (struct base_device *)modules[i];

        if (!device || device->module.type != BASE_TYPE_DEVICE)
            continue;

        if (device->driver)
            continue;

        if (!driver->check(driver, device))
            continue;

        device->driver = driver;

        if (driver->attach)
            driver->attach(device);

        found = 1;

    }

    return found;

}

static void register_module(struct base_module *module)
{

    unsigned int i;

    for (i = 0; i < BASE_MODULE_SLOTS; i++)
    {

        if (modules[i])
            continue;

        modules[i] = module;

        break;

    }

}

void base_register_bus(struct base_bus *bus)
{

    register_module(&bus->module);

    if (bus->scan)
        bus->scan(bus);

}

void base_register_device(struct base_device *device)
{

    register_module(&device->module);

}

void base_register_driver(struct base_driver *driver)
{

    register_module(&driver->module);

    if (driver->check)
    {

        if (!attach(driver))
            return;

        if (driver->start)
            driver->start(driver);

    }

    else
    {

        if (driver->start)
            driver->start(driver);

    }

}

static void unregister_module(struct base_module *module)
{

    unsigned int i;

    for (i = 0; i < BASE_MODULE_SLOTS; i++)
    {

        if (!modules[i])
            continue;

        if (modules[i] != module)
            continue;

        modules[i] = 0;

        break;

    }

}

void base_unregister_bus(struct base_bus *bus)
{

    unregister_module(&bus->module);

}

void base_unregister_device(struct base_device *device)
{

    unregister_module(&device->module);

}

void base_unregister_driver(struct base_driver *driver)
{

    unregister_module(&driver->module);

}

static void base_init_module(struct base_module *module, unsigned int type, char *name)
{

    memory_clear(module, sizeof (struct base_module));

    module->type = type;
    module->name = name;

}

void base_init_bus(struct base_bus *bus, unsigned int type, char *name, void (*scan)(struct base_bus *self))
{

    memory_clear(bus, sizeof (struct base_bus));

    base_init_module(&bus->module, BASE_TYPE_BUS, name);

    bus->type = type;
    bus->scan = scan;

}

void base_init_device(struct base_device *device, unsigned int type, char *name)
{

    memory_clear(device, sizeof (struct base_device));

    base_init_module(&device->module, BASE_TYPE_DEVICE, name);

    device->type = type;

}

void base_init_driver(struct base_driver *driver, unsigned int type, char *name, void (*start)(struct base_driver *self), unsigned int (*check)(struct base_driver *self, struct base_device *device), void (*attach)(struct base_device *device))
{

    memory_clear(driver, sizeof (struct base_driver));

    base_init_module(&driver->module, BASE_TYPE_DRIVER, name);

    driver->type = type;
    driver->start = start;
    driver->check = check;
    driver->attach = attach;

}

void init()
{

    memory_clear(modules, sizeof (struct base_module *) * BASE_MODULE_SLOTS);

}

void destroy()
{

}

