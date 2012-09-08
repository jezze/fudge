#include <memory.h>
#include <base/base.h>
#include <arch/x86/apic/apic.h>
#include <arch/x86/io/io.h>
#include <arch/x86/pit/pit.h>

static void handle_irq(struct base_device *self)
{

    struct pit_device *device = (struct pit_device *)self;

    device->jiffies += 1;

}

static void attach(struct base_device *device)
{

    struct pit_device *pitDevice = (struct pit_device *)device;

    io_outb(0x43, 0x36);
    io_outb(0x40, (unsigned char)(pitDevice->divisor & 0xFF));
    io_outb(0x40, (unsigned char)((pitDevice->divisor >> 8) & 0xFF));

    apic_register_routine(pitDevice->irq, device, handle_irq);

}

static unsigned int check(struct base_driver *self, struct base_device *device)
{

    return device->type == PIT_DEVICE_TYPE;

}

void pit_driver_init(struct pit_driver *driver)
{

    memory_clear(driver, sizeof (struct pit_driver));

    base_driver_init(&driver->base, PIT_DRIVER_TYPE, "pit", 0, check, attach);

}

