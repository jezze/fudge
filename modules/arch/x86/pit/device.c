#include <lib/memory.h>
#include <kernel/arch/x86/io.h>
#include <kernel/modules.h>
#include <modules/pit/pit.h>

void pit_device_init(struct pit_device *device, unsigned int irq)
{

    memory_clear(device, sizeof (struct pit_device));

    modules_device_init(&device->base, PIT_DEVICE_TYPE, "pit");

    device->irq = irq;
    device->divisor = PIT_HERTZ / PIT_FREQUENCY;

    io_outb(0x43, 0x36);
    io_outb(0x40, (unsigned char)(device->divisor & 0xFF));
    io_outb(0x40, (unsigned char)((device->divisor >> 8) & 0xFF));

}

