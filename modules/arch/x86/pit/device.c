#include <fudge/module.h>
#include <kernel/vfs.h>
#include <kernel/kernel.h>
#include <base/base.h>
#include <base/timer.h>
#include "pit.h"

void pit_init_device(struct pit_device *device, unsigned int irq)
{

    memory_clear(device, sizeof (struct pit_device));
    base_init_device(&device->base, PIT_DEVICE_TYPE, irq, "pit", 0);
    memory_write_paddednumber(device->base.module.name, 13, irq, 16, 4, 9);

}

