#include <memory.h>
#include <base/base.h>
#include <arch/x86/ps2/ps2.h>

void ps2_mouse_device_init(struct ps2_mouse_device *device, unsigned int irq)
{

    memory_clear(device, sizeof (struct ps2_mouse_device));

    base_device_init(&device->base, PS2_MOUSE_DEVICE_TYPE, "mouse");

    device->irq = irq;

}

