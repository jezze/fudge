#include <fudge/module.h>
#include <system/system.h>
#include "base.h"
#include "keyboard.h"
#include "mouse.h"
#include "network.h"
#include "terminal.h"
#include "timer.h"
#include "video.h"

static struct base_module *modules;
static struct system_group root;

static void register_module(struct base_module *module)
{

    struct base_module *current;

    if (!modules)
    {

        modules = module;

        return;

    }

    for (current = modules; current; current = current->sibling)
    {

        if (current->sibling)
            continue;

        current->sibling = module;

        return;

    }

}

void base_register_bus(struct base_bus *bus)
{

    register_module(&bus->module);

    bus->scan(bus);

}

void base_register_device(struct base_device *device)
{

    register_module(&device->module);

}

void base_register_driver(struct base_driver *driver)
{

    struct base_module *current;

    register_module(&driver->module);

    for (current = modules; current; current = current->sibling)
    {

        struct base_device *device = (struct base_device *)current;

        if (current->type != BASE_TYPE_DEVICE)
            continue;

        if (device->driver)
            continue;

        if (!driver->check(device))
            continue;

        device->driver = driver;
        device->driver->attach(device);

    }

}

static void unregister_module(struct base_module *module)
{

    struct base_module *current;

    if (modules == module)
    {

        modules = modules->sibling;

        return;

    }

    for (current = modules; current; current = current->sibling)
    {

        if (current->sibling != module)
            continue;

        current->sibling = current->sibling->sibling;

        return;

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

static void base_init_module(struct base_module *module, enum base_type type, char *name)
{

    memory_clear(module, sizeof (struct base_module));
    system_init_group(&module->base, name);

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

void base_init_device(struct base_device *device, unsigned int type, unsigned int irq, char *name, struct base_bus *bus)
{

    memory_clear(device, sizeof (struct base_device));
    base_init_module(&device->module, BASE_TYPE_DEVICE, name);

    device->type = type;
    device->irq = irq;
    device->bus = bus;

}

void base_init_driver(struct base_driver *driver, char *name, unsigned int (*check)(struct base_device *device), void (*attach)(struct base_device *device))
{

    memory_clear(driver, sizeof (struct base_driver));
    base_init_module(&driver->module, BASE_TYPE_DRIVER, name);

    driver->check = check;
    driver->attach = attach;

}

void init()
{

    modules = 0;

    system_init_group(&root, "base");
    system_register_node(&root.node);

    base_setup_keyboard();
    base_setup_mouse();
    base_setup_network();
    base_setup_terminal();
    base_setup_timer();
    base_setup_video();

}

void destroy()
{

    system_unregister_node(&root.node);

}

