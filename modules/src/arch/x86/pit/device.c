#include <memory.h>
#include <base/base.h>
#include <arch/x86/pit/pit.h>

void pit_init_device(struct pit_device *device, unsigned int irq)
{

    memory_clear(device, sizeof (struct pit_device));

    base_init_device(&device->base, PIT_DEVICE_TYPE, "pit");

    device->irq = irq;
    device->divisor = PIT_FREQUENCY / PIT_HERTZ;

}

