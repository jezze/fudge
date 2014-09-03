#include <module.h>
#include <kernel/resource.h>
#include <system/system.h>
#include "base.h"
#include "block.h"
#include "clock.h"
#include "keyboard.h"
#include "mouse.h"
#include "network.h"
#include "terminal.h"
#include "timer.h"
#include "video.h"

void base_registerbus(struct base_bus *bus)
{

    resource_register(&bus->resource);

    bus->setup(bus);

}

void base_registerdriver(struct base_driver *driver)
{

    struct resource *current = 0;

    while ((current = resource_findtype(current, BASE_RESOURCE_BUS)))
    {

        struct base_bus *bus = current->data;
        unsigned int id = 0;

        while ((id = bus->next(bus, id)))
        {

            if (!driver->match(bus, id))
                continue;

            driver->attach(bus, id);

        }

    }

    resource_register(&driver->resource);

}

void base_registerinterface(struct base_interface *interface)
{

    resource_register(&interface->resource);

}

void base_unregisterbus(struct base_bus *bus)
{

    resource_unregister(&bus->resource);

}

void base_unregisterdriver(struct base_driver *driver)
{

    struct resource *current = 0;

    while ((current = resource_findtype(current, BASE_RESOURCE_BUS)))
    {

        struct base_bus *bus = current->data;
        unsigned int id = 0;

        while ((id = bus->next(bus, id)))
        {

            if (!driver->match(bus, id))
                continue;

            driver->detach(bus, id);

        }

    }

    resource_unregister(&driver->resource);

}

void base_unregisterinterface(struct base_interface *interface)
{

    resource_unregister(&interface->resource);

}

void base_initbus(struct base_bus *bus, unsigned int type, const char *name, void (*setup)(struct base_bus *self), unsigned int (*next)(struct base_bus *self, unsigned int id), unsigned short (*irq)(struct base_bus *self, unsigned int id))
{

    memory_clear(bus, sizeof (struct base_bus));
    resource_init(&bus->resource, BASE_RESOURCE_BUS, bus);

    bus->type = type;
    bus->name = name;
    bus->setup = setup;
    bus->next = next;
    bus->irq = irq;

}

void base_initdriver(struct base_driver *driver, const char *name, unsigned int (*match)(struct base_bus *bus, unsigned int id), void (*attach)(struct base_bus *bus, unsigned int id), void (*detach)(struct base_bus *bus, unsigned int id))
{

    memory_clear(driver, sizeof (struct base_driver));
    resource_init(&driver->resource, BASE_RESOURCE_DRIVER, driver);

    driver->name = name;
    driver->match = match;
    driver->attach = attach;
    driver->detach = detach;

}

void base_initinterface(struct base_interface *interface, unsigned int type)
{

    memory_clear(interface, sizeof (struct base_interface));
    resource_init(&interface->resource, BASE_RESOURCE_INTERFACE, interface);

    interface->type = type;

}

void base_initdevice(struct base_device *device, struct base_bus *bus, unsigned int id)
{

    memory_clear(device, sizeof (struct base_device));
    resource_init(&device->resource, BASE_RESOURCE_DEVICE, device);

    device->bus = bus;
    device->id = id;

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

