#include <module.h>
#include <kernel/resource.h>
#include <base/base.h>
#include "platform.h"

void platform_init_device(struct platform_device *device, unsigned int type, char *name, struct platform_bus *bus, unsigned short registers)
{

    memory_clear(device, sizeof (struct platform_device));
    base_init_device(&device->base, type, name, &bus->base);

    device->registers = registers;

}

