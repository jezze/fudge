#include <fudge/module.h>
#include "base.h"
#include "block.h"
#include "clock.h"
#include "keyboard.h"
#include "mouse.h"
#include "network.h"
#include "terminal.h"
#include "timer.h"
#include "video.h"

static struct list modules;

void base_register_bus(struct base_bus *bus)
{

    list_add(&modules, &bus->module.item);

    bus->scan(bus);

}

void base_register_device(struct base_device *device)
{

    list_add(&modules, &device->module.item);

}

void base_register_driver(struct base_driver *driver)
{

    struct list_item *current;

    list_add(&modules, &driver->module.item);

    for (current = modules.head; current; current = current->next)
    {

        struct base_module *module = current->data;
        struct base_device *device = current->data;

        if (module->type != BASE_TYPE_DEVICE)
            continue;

        if (device->driver)
            continue;

        if (!driver->check(device))
            continue;

        device->driver = driver;
        device->driver->attach(device);

    }

}

void base_unregister_bus(struct base_bus *bus)
{

    list_remove(&modules, &bus->module.item);

}

void base_unregister_device(struct base_device *device)
{

    list_remove(&modules, &device->module.item);

}

void base_unregister_driver(struct base_driver *driver)
{

    list_remove(&modules, &driver->module.item);

}

static void base_init_module(struct base_module *module, enum base_type type, const char *name)
{

    memory_clear(module, sizeof (struct base_module));
    list_init_item(&module->item, module);

    module->type = type;
    module->name = name;

}

void base_init_bus(struct base_bus *bus, unsigned int type, const char *name, void (*scan)(struct base_bus *self))
{

    memory_clear(bus, sizeof (struct base_bus));
    base_init_module(&bus->module, BASE_TYPE_BUS, name);

    bus->type = type;
    bus->scan = scan;

}

void base_init_device(struct base_device *device, unsigned int type, unsigned int irq, const char *name, struct base_bus *bus)
{

    memory_clear(device, sizeof (struct base_device));
    base_init_module(&device->module, BASE_TYPE_DEVICE, name);

    device->type = type;
    device->irq = irq;
    device->bus = bus;

}

void base_init_driver(struct base_driver *driver, const char *name, unsigned int (*check)(struct base_device *device), void (*attach)(struct base_device *device))
{

    memory_clear(driver, sizeof (struct base_driver));
    base_init_module(&driver->module, BASE_TYPE_DRIVER, name);

    driver->check = check;
    driver->attach = attach;

}

void base_init_interface(struct base_interface *interface, unsigned int type)
{

    memory_clear(interface, sizeof (struct base_interface));

    interface->type = type;

}

void init()
{

    list_init(&modules);
    base_block_setup();
    base_clock_setup();
    base_keyboard_setup();
    base_mouse_setup();
    base_network_setup();
    base_terminal_setup();
    base_timer_setup();
    base_video_setup();

}

void destroy()
{

}

