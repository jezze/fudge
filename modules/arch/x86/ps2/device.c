#include <fudge/define.h>
#include <fudge/memory.h>
#include <fudge/data/circular.h>
#include <system/system.h>
#include <base/base.h>
#include <kbd/kbd.h>
#include <mouse/mouse.h>
#include "ps2.h"

void ps2_init_device(struct ps2_device *device, char *name, unsigned int irq)
{

    memory_clear(device, sizeof (struct ps2_device));

    base_init_device(&device->base, PS2_DEVICE_TYPE, name);

    device->irq = irq;

}

