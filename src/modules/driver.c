#include <fudge.h>
#include <kernel.h>
#include "bus.h"
#include "driver.h"

void base_registerdriver(struct base_driver *driver, unsigned int type)
{

    struct resource *current = 0;

    while ((current = resource_foreachtype(current, RESOURCE_BUS)))
    {

        struct base_bus *bus = current->data;

        if (bus->type == type)
        {

            unsigned int id = 0;

            while ((id = bus->next(id)))
            {

                if (driver->match(id))
                {

                    driver->init(id);
                    driver->reset(id);
                    driver->attach(id);

                }

            }

        }

    }

    resource_register(&driver->resource);

}

void base_unregisterdriver(struct base_driver *driver, unsigned int type)
{

    struct resource *current = 0;

    while ((current = resource_foreachtype(current, RESOURCE_BUS)))
    {

        struct base_bus *bus = current->data;

        if (bus->type == type)
        {

            unsigned int id = 0;

            while ((id = bus->next(id)))
            {

                if (driver->match(id))
                    driver->detach(id);

            }

        }

    }

    resource_unregister(&driver->resource);

}

void base_initdriver(struct base_driver *driver, char *name, void (*init)(unsigned int id), unsigned int (*match)(unsigned int id), void (*reset)(unsigned int id), void (*attach)(unsigned int id), void (*detach)(unsigned int id))
{

    resource_init(&driver->resource, RESOURCE_DRIVER, driver);

    driver->name = name;
    driver->init = init;
    driver->match = match;
    driver->reset = reset;
    driver->attach = attach;
    driver->detach = detach;

}

