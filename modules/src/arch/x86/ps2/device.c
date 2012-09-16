#include <memory.h>
#include <base/base.h>
#include <arch/x86/ps2/ps2.h>

void ps2_device_init(struct ps2_device *device, char *name, unsigned int irq)
{

    memory_clear(device, sizeof (struct ps2_device));

    base_device_init(&device->base, PS2_DEVICE_TYPE, name);

    device->irq = irq;

}

