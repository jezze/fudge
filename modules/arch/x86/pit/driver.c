#include <fudge/module.h>
#include <system/system.h>
#include <base/base.h>
#include <arch/x86/pic/pic.h>
#include <arch/x86/io/io.h>
#include "pit.h"

static void handle_irq(struct base_device *device)
{

    struct pit_device *pitDevice = (struct pit_device *)device;

    pitDevice->jiffies += 1;

}

static void attach(struct base_device *device)
{

    struct pit_device *pitDevice = (struct pit_device *)device;

    pic_set_routine(device->irq, device, handle_irq);
    io_outb(PIT_COMMAND, PIT_COMMAND_COUNTER0 | PIT_COMMAND_BOTH | PIT_COMMAND_MODE3 | PIT_COMMAND_BINARY);
    io_outb(PIT_COUNTER0, (unsigned char)(pitDevice->divisor >> 0));
    io_outb(PIT_COUNTER0, (unsigned char)(pitDevice->divisor >> 8));

}

static unsigned int check(struct base_device *device)
{

    return device->type == PIT_DEVICE_TYPE;

}

void pit_init_driver(struct pit_driver *driver)
{

    memory_clear(driver, sizeof (struct pit_driver));
    base_init_driver(&driver->base, "pit", 0, check, attach);

}

