#include <fudge.h>
#include <kernel.h>
#include "bus.h"
#include "driver.h"

/* a device belongs to the first driver that matches it and is offered to no other; a driver drives one device */
static unsigned int isowned(unsigned int type, unsigned int id)
{

    struct resource *current = 0;

    while ((current = resource_foreachtype(current, RESOURCE_DRIVER)))
    {

        struct base_driver *driver = current->data;

        if (driver->type == type && driver->id == id)
            return 1;

    }

    return 0;

}

static void attach(struct base_driver *driver, unsigned int type, unsigned int id)
{

    driver->type = type;
    driver->id = id;

    driver->init(id);
    driver->reset(id);
    driver->attach(id);

}

static void detach(struct base_driver *driver)
{

    driver->detach(driver->id);

    driver->type = 0;
    driver->id = 0;

}

/* every bus is offered: a driver's match checks the bus type itself */
void base_registerdriver(struct base_driver *driver)
{

    struct resource *current = 0;

    while (!driver->id && (current = resource_foreachtype(current, RESOURCE_BUS)))
    {

        struct base_bus *bus = current->data;
        unsigned int id = 0;

        while (!driver->id && (id = bus->next(id)))
        {

            if (!isowned(bus->type, id) && driver->match(bus->type, id))
                attach(driver, bus->type, id);

        }

    }

    resource_register(&driver->resource);

}

void base_unregisterdriver(struct base_driver *driver)
{

    if (driver->id)
        detach(driver);

    resource_unregister(&driver->resource);

}

void base_attachdevice(unsigned int type, unsigned int id)
{

    struct resource *current = 0;

    if (isowned(type, id))
        return;

    while ((current = resource_foreachtype(current, RESOURCE_DRIVER)))
    {

        struct base_driver *driver = current->data;

        if (!driver->id && driver->match(type, id))
        {

            attach(driver, type, id);

            break;

        }

    }

}

void base_detachdevice(unsigned int type, unsigned int id)
{

    struct resource *current = 0;

    while ((current = resource_foreachtype(current, RESOURCE_DRIVER)))
    {

        struct base_driver *driver = current->data;

        if (driver->type == type && driver->id == id)
            detach(driver);

    }

}

void base_initdriver(struct base_driver *driver, char *name, void (*init)(unsigned int id), unsigned int (*match)(unsigned int type, unsigned int id), void (*reset)(unsigned int id), void (*attach)(unsigned int id), void (*detach)(unsigned int id))
{

    resource_init(&driver->resource, RESOURCE_DRIVER, driver);

    driver->name = name;
    driver->type = 0;
    driver->id = 0;
    driver->init = init;
    driver->match = match;
    driver->reset = reset;
    driver->attach = attach;
    driver->detach = detach;

}

