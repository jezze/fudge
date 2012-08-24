#include <memory.h>
#include <modules/modules.h>

static struct modules_base *modules[MODULES_MODULE_SLOTS];

static void attach(struct modules_driver *driver)
{

    unsigned int i;

    if (!driver->check)
        return;

    for (i = 0; i < MODULES_MODULE_SLOTS; i++)
    {

        struct modules_device *device = (struct modules_device *)modules[i];

        if (!device || device->base.type != MODULES_TYPE_DEVICE)
            continue;

        if (device->driver)
            continue;

        if (!driver->check(driver, device))
            continue;

        device->driver = driver;

        if (driver->attach)
            driver->attach(device);

    }

}

static void register_base(struct modules_base *base)
{

    unsigned int i;

    for (i = 0; i < MODULES_MODULE_SLOTS; i++)
    {

        if (modules[i])
            continue;

        modules[i] = base;

        break;

    }

}

void modules_register_bus(struct modules_bus *bus)
{

    register_base(&bus->base);

    if (bus->scan)
        bus->scan(bus);

}

void modules_register_device(struct modules_device *device)
{

    register_base(&device->base);

}

void modules_register_driver(struct modules_driver *driver)
{

    register_base(&driver->base);
    attach(driver);

    if (driver->start)
        driver->start(driver);

}

static void unregister_base(struct modules_base *base)
{

    unsigned int i;

    for (i = 0; i < MODULES_MODULE_SLOTS; i++)
    {

        if (!modules[i])
            continue;

        if (modules[i] != base)
            continue;

        modules[i] = 0;

        break;

    }

}

void modules_unregister_bus(struct modules_bus *bus)
{

    unregister_base(&bus->base);

}

void modules_unregister_device(struct modules_device *device)
{

    unregister_base(&device->base);

}

void modules_unregister_driver(struct modules_driver *driver)
{

    unregister_base(&driver->base);

}

static void base_init(struct modules_base *base, unsigned int type, char *name)
{

    memory_clear(base, sizeof (struct modules_base));

    base->type = type;
    base->name = name;

}

void modules_bus_init(struct modules_bus *bus, unsigned int type, char *name, void (*scan)(struct modules_bus *self))
{

    memory_clear(bus, sizeof (struct modules_bus));

    base_init(&bus->base, MODULES_TYPE_BUS, name);

    bus->type = type;
    bus->scan = scan;

}

void modules_device_init(struct modules_device *device, unsigned int type, char *name)
{

    memory_clear(device, sizeof (struct modules_device));

    base_init(&device->base, MODULES_TYPE_DEVICE, name);

    device->type = type;

}

void modules_driver_init(struct modules_driver *driver, unsigned int type, char *name, void (*start)(struct modules_driver *self), unsigned int (*check)(struct modules_driver *self, struct modules_device *device), void (*attach)(struct modules_device *device))
{

    memory_clear(driver, sizeof (struct modules_driver));

    base_init(&driver->base, MODULES_TYPE_DRIVER, name);

    driver->type = type;
    driver->start = start;
    driver->check = check;
    driver->attach = attach;

}

void init()
{

    memory_clear(modules, sizeof (struct modules_base *) * MODULES_MODULE_SLOTS);

}

void destroy()
{

}
