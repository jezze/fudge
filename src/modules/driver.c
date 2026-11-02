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

static void attach(struct base_driver *driver, unsigned int id)
{

    driver->id = id;

    driver->init(id);
    driver->reset(id);
    driver->attach(id);

}

static void detach(struct base_driver *driver)
{

    driver->detach(driver->id);

    driver->id = 0;

}

void base_registerdriver(struct base_driver *driver, unsigned int type)
{

    struct resource *current = 0;

    while ((current = resource_foreachtype(current, RESOURCE_BUS)))
    {

        struct base_bus *bus = current->data;

        if (bus->type == type)
        {

            unsigned int id = 0;

            while (!driver->id && (id = bus->next(id)))
            {

                if (!isowned(type, id) && driver->match(id))
                    attach(driver, id);

            }

        }

    }

    driver->type = type;

    resource_register(&driver->resource);

}

void base_unregisterdriver(struct base_driver *driver, unsigned int type)
{

    if (driver->id)
        detach(driver);

    driver->type = 0;

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

        if (driver->type == type && !driver->id && driver->match(id))
        {

            attach(driver, id);

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

void base_initdriver(struct base_driver *driver, char *name, void (*init)(unsigned int id), unsigned int (*match)(unsigned int id), void (*reset)(unsigned int id), void (*attach)(unsigned int id), void (*detach)(unsigned int id))
{

    resource_init(&driver->resource, RESOURCE_DRIVER, driver);

    driver->name = name;
    driver->id = 0;
    driver->init = init;
    driver->match = match;
    driver->reset = reset;
    driver->attach = attach;
    driver->detach = detach;

}

