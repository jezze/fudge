#include <fudge/module.h>
#include <base/base.h>
#include "platform.h"

static void scan(struct base_bus *self)
{

}

void platform_init_bus(struct platform_bus *bus)
{

    memory_clear(bus, sizeof (struct platform_bus));
    base_init_bus(&bus->base, PLATFORM_BUS_TYPE, "platform", scan);

}

