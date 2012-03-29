#include <lib/memory.h>
#include <kernel/modules.h>
#include <modules/ps2/ps2.h>

void kbd_device_init(struct kbd_device *device, unsigned int irq)
{

    memory_clear(device, sizeof (struct kbd_device));

    modules_device_init(&device->base, KBD_DEVICE_TYPE, "kbd");

    device->irq = irq;

}

