#include <memory.h>
#include <base/base.h>
#include <arch/x86/ps2/ps2.h>

void ps2_kbd_device_init(struct ps2_kbd_device *device, unsigned int irq)
{

    memory_clear(device, sizeof (struct ps2_kbd_device));

    base_device_init(&device->base, PS2_KBD_DEVICE_TYPE, "kbd");

    device->irq = irq;

}

