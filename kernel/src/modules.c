#include <memory.h>
#include <string.h>
#include <modules.h>
#include <vfs/root.h>
#include <vfs/sys.h>

static union modules_module *modules[MODULES_MODULE_SLOTS];

static void attach(struct modules_driver *driver)
{

    unsigned int i;

    if (!driver->check)
        return;

    for (i = 0; i < MODULES_MODULE_SLOTS; i++)
    {

        union modules_module *module = modules[i];

        if (!module || module->base.type != MODULES_TYPE_DEVICE)
            continue;

        if (module->device.driver)
            continue;

        if (!driver->check(driver, &module->device))
            continue;

        module->device.driver = driver;

        if (driver->attach)
            driver->attach(&module->device);

    }

}

struct modules_filesystem *modules_get_filesystem(char *path)
{

    unsigned int i;
    unsigned int max = 0;
    struct modules_filesystem *current = 0;

    for (i = 0; i < MODULES_MODULE_SLOTS; i++)
    {

        unsigned int length;
        union modules_module *module = modules[i];

        if (!module || module->base.type != MODULES_TYPE_FILESYSTEM)
            continue;

        length = string_length(module->filesystem.path);

        if (!memory_compare(module->filesystem.path, path, length))
            continue;

        if (length < max)
            continue;

        current = &module->filesystem;
        max = length;

    }

    return current;

}

static void register_base(struct modules_base *base)
{

    unsigned int i;
    union modules_module *module = (union modules_module *)base;

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

void modules_register_filesystem(struct modules_filesystem *filesystem)
{

    register_base(&filesystem->base);

}

static void unregister_base(struct modules_base *base)
{

    unsigned int i;
    union modules_module *module = (union modules_module *)base;

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

void modules_unregister_filesystem(struct modules_filesystem *filesystem)
{

    unregister_base(&filesystem->base);

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

void modules_filesystem_init(struct modules_filesystem *filesystem, unsigned int type, char *name, void (*open)(struct modules_filesystem *self, unsigned int id), void (*close)(struct modules_filesystem *self, unsigned int id), unsigned int (*read)(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*walk)(struct modules_filesystem *self, unsigned int id, unsigned int count, char *path), unsigned int (*get_physical)(struct modules_filesystem *self, unsigned int id))
{

    memory_clear(filesystem, sizeof (struct modules_filesystem));

    base_init(&filesystem->base, MODULES_TYPE_FILESYSTEM, name);

    filesystem->type = type;
    filesystem->open = open;
    filesystem->close = close;
    filesystem->read = read;
    filesystem->write = write;
    filesystem->walk = walk;
    filesystem->get_physical = get_physical;

}

void modules_init()
{

    vfs_root_init(modules);
    vfs_sys_init(modules);

}

