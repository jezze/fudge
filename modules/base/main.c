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

    resource_register(&bus->resource);

    bus->setup(bus);

}

void base_register_device(struct base_device *device)
{

    resource_register(&device->resource);

}

void base_register_driver(struct base_driver *driver)
{

    struct resource *current = 0;

    while ((current = resource_find(current)))
    {

        struct base_device *device = current->data;

        if (current->type != BASE_RESOURCE_DEVICE)
            continue;

        if (!driver->check(device->bus, device->type))
            continue;

        driver->attach(device->bus, device->type);

    }

    resource_register(&driver->resource);

}

void base_unregister_bus(struct base_bus *bus)
{

    resource_unregister(&bus->resource);

}

void base_unregister_device(struct base_device *device)
{

    resource_unregister(&device->resource);

}

void base_unregister_driver(struct base_driver *driver)
{

    struct resource *current = 0;

    while ((current = resource_find(current)))
    {

        struct base_device *device = current->data;

        if (current->type != BASE_RESOURCE_DEVICE)
            continue;

        if (!driver->check(device->bus, device->type))
            continue;

        driver->detach(device->bus, device->type);

    }

    resource_unregister(&driver->resource);

}

void base_init_bus(struct base_bus *bus, unsigned int type, const char *name, void (*setup)(struct base_bus *self), unsigned int (*device_next)(struct base_bus *self, unsigned int id), unsigned short (*device_irq)(struct base_bus *self, unsigned int id))
{

    memory_clear(bus, sizeof (struct base_bus));
    resource_init(&bus->resource, BASE_RESOURCE_BUS, bus);

    bus->type = type;
    bus->name = name;
    bus->setup = setup;
    bus->device_next = device_next;
    bus->device_irq = device_irq;

}

void base_init_device(struct base_device *device, unsigned int type, const char *name, struct base_bus *bus)
{

    memory_clear(device, sizeof (struct base_device));
    resource_init(&device->resource, BASE_RESOURCE_DEVICE, device);

    device->type = type;
    device->name = name;
    device->bus = bus;

}

void base_init_driver(struct base_driver *driver, const char *name, unsigned int (*check)(struct base_bus *bus, unsigned int id), void (*attach)(struct base_bus *bus, unsigned int id), void (*detach)(struct base_bus *bus, unsigned int id))
{

    memory_clear(driver, sizeof (struct base_driver));
    resource_init(&driver->resource, BASE_RESOURCE_DRIVER, driver);

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

