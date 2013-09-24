#include <fudge/module.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include "base.h"
#include "block.h"
#include "keyboard.h"
#include "mouse.h"
#include "network.h"
#include "terminal.h"
#include "timer.h"
#include "video.h"

static struct base_module *modules;
static struct system_group root;

static struct base_module *find_presibling(struct base_module *module)
{

    struct base_module *current = modules;

    if (current == module)
        return 0;

    while (current->sibling != module)
        current = current->sibling;

    return current;

}

static void register_module(struct base_module *module)
{

    struct base_module *current = find_presibling(0);

    if (current)
        current->sibling = module;
    else
        modules = module;

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

    struct base_module *current = find_presibling(module);

    if (current)
        current->sibling = current->sibling->sibling;
    else
        modules = modules->sibling;

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

    base_block_setup();
    base_keyboard_setup();
    base_mouse_setup();
    base_network_setup();
    base_terminal_setup();
    base_timer_setup();
    base_video_setup();

}

void destroy()
{

    system_unregister_node(&root.node);

}

