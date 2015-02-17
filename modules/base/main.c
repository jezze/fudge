#include <fudge.h>
#include <kernel.h>
#include <system/system.h>
#include "base.h"

void base_registerbus(struct base_bus *bus)
{

    resource_register(&bus->resource);

    bus->setup();

}

void base_registerdriver(struct base_driver *driver, unsigned int type)
{

    struct resource *current = 0;

    while ((current = resource_findtype(current, RESOURCE_TYPE_BUS)))
    {

        struct base_bus *bus = current->data;
        unsigned int id = 0;

        if (bus->type != type)
            continue;

        while ((id = bus->next(id)))
        {

            if (!driver->match(id))
                continue;

            driver->init();
            driver->attach(id);

        }

    }

    resource_register(&driver->resource);

}

void base_registerinterface(struct base_interface *interface, unsigned int id)
{

    resource_register(&interface->resource);

    interface->id = id;

}

void base_unregisterbus(struct base_bus *bus)
{

    resource_unregister(&bus->resource);

}

void base_unregisterdriver(struct base_driver *driver, unsigned int type)
{

    struct resource *current = 0;

    while ((current = resource_findtype(current, RESOURCE_TYPE_BUS)))
    {

        struct base_bus *bus = current->data;
        unsigned int id = 0;

        if (bus->type != type)
            continue;

        while ((id = bus->next(id)))
        {

            if (!driver->match(id))
                continue;

            driver->detach(id);

        }

    }

    resource_unregister(&driver->resource);

}

void base_unregisterinterface(struct base_interface *interface)
{

    resource_unregister(&interface->resource);

}

void base_initbus(struct base_bus *bus, unsigned int type, const char *name, void (*setup)(), unsigned int (*next)(unsigned int id))
{

    resource_init(&bus->resource, RESOURCE_TYPE_BUS, bus);

    bus->type = type;
    bus->name = name;
    bus->setup = setup;
    bus->next = next;

}

void base_initdriver(struct base_driver *driver, const char *name, void (*init)(), unsigned int (*match)(unsigned int id), void (*attach)(unsigned int id), void (*detach)(unsigned int id))
{

    resource_init(&driver->resource, RESOURCE_TYPE_DRIVER, driver);

    driver->name = name;
    driver->init = init;
    driver->match = match;
    driver->attach = attach;
    driver->detach = detach;

}

void base_initinterface(struct base_interface *interface, struct base_driver *driver)
{

    resource_init(&interface->resource, RESOURCE_TYPE_INTERFACE, interface);

    interface->driver = driver;

}

void module_init()
{

}

void module_register()
{

}

void module_unregister()
{

}

