#include <kernel/modules.h>
#include <kernel/kernel.h>
#include <modules/io/io.h>
#include <modules/pit/pit.h>

static struct pit_device pitDevice;

static void pit_handler()
{

}

void pit_device_init(struct pit_device *device)
{

    device->base.module.type = MODULES_TYPE_DEVICE;
    device->base.type = PIT_DEVICE_TYPE;
    device->divisor = PIT_HERTZ / PIT_FREQUENCY;

    io_outb(0x43, 0x36);
    io_outb(0x40, (unsigned char)(device->divisor & 0xFF));
    io_outb(0x40, (unsigned char)((device->divisor >> 8) & 0xFF));

}

void pit_init()
{

    pit_device_init(&pitDevice);

    kernel_register_irq(0x00, pit_handler);

    modules_register_device(&pitDevice.base);

}

