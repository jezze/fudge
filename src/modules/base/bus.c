#include <fudge.h>
#include <kernel.h>
#include "bus.h"

void base_registerbus(struct base_bus *bus)
{

    resource_register(&bus->resource);

    bus->setup();

}

void base_unregisterbus(struct base_bus *bus)
{

    resource_unregister(&bus->resource);

}

void base_initbus(struct base_bus *bus, unsigned int type, char *name, void (*setup)(void), unsigned int (*next)(unsigned int id))
{

    resource_init(&bus->resource, RESOURCE_BUS, bus);

    bus->type = type;
    bus->name = name;
    bus->setup = setup;
    bus->next = next;

}

