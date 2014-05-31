#include <module.h>
#include <kernel/resource.h>
#include <base/base.h>
#include "platform.h"

void platform_init_device(struct platform_device *device, unsigned int type, unsigned int irq, char *name, struct platform_bus *bus, unsigned short registers)
{

    memory_clear(device, sizeof (struct platform_device));
    base_init_device(&device->base, type, irq, name, &bus->base);

    device->registers = registers;

}

