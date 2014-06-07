#include <module.h>
#include <kernel/resource.h>
#include <kernel/rendezvous.h>
#include "base.h"
#include "block.h"
#include "clock.h"
#include "keyboard.h"
#include "mouse.h"
#include "network.h"
#include "terminal.h"
#include "timer.h"
#include "video.h"

void base_register_bus(struct base_bus *bus)
{

    resource_register_item(&bus->resource);

    bus->scan(bus);

}

void base_register_device(struct base_device *device)
{

    resource_register_item(&device->resource);

}

void base_register_driver(struct base_driver *driver)
{

    struct resource_item *current = 0;

    while ((current = resource_find_item(current)))
    {

        struct base_device *device = current->data;

        if (current->type != BASE_RESOURCE_DEVICE)
            continue;

        if (!driver->check(device))
            continue;

        device->driver = driver;
        device->driver->attach(device);

    }

    resource_register_item(&driver->resource);

}

void base_unregister_bus(struct base_bus *bus)
{

    resource_unregister_item(&bus->resource);

}

void base_unregister_device(struct base_device *device)
{

    resource_unregister_item(&device->resource);

}

void base_unregister_driver(struct base_driver *driver)
{

    struct resource_item *current = 0;

    while ((current = resource_find_item(current)))
    {

        struct base_device *device = current->data;

        if (current->type != BASE_RESOURCE_DEVICE)
            continue;

        if (device->driver != driver)
            continue;

        device->driver->detach(device);
        device->driver = 0;

    }

    resource_unregister_item(&driver->resource);

}

void base_init_bus(struct base_bus *bus, unsigned int type, const char *name, void (*scan)(struct base_bus *self))
{

    memory_clear(bus, sizeof (struct base_bus));
    resource_init_item(&bus->resource, BASE_RESOURCE_BUS, bus);

    bus->name = name;
    bus->type = type;
    bus->scan = scan;

}

void base_init_device(struct base_device *device, unsigned int type, unsigned int irq, const char *name, struct base_bus *bus)
{

    memory_clear(device, sizeof (struct base_device));
    resource_init_item(&device->resource, BASE_RESOURCE_DEVICE, device);

    device->name = name;
    device->type = type;
    device->irq = irq;
    device->bus = bus;

}

void base_init_driver(struct base_driver *driver, const char *name, unsigned int (*check)(struct base_device *device), void (*attach)(struct base_device *device), void (*detach)(struct base_device *device))
{

    memory_clear(driver, sizeof (struct base_driver));
    resource_init_item(&driver->resource, BASE_RESOURCE_DRIVER, driver);

    driver->name = name;
    driver->check = check;
    driver->attach = attach;
    driver->detach = detach;

}

void base_init_interface(struct base_interface *interface, unsigned int type)
{

    memory_clear(interface, sizeof (struct base_interface));

    interface->type = type;

}

void init()
{

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

