#include <kernel/arch/x86/io.h>
#include <kernel/event.h>
#include <kernel/irq.h>
#include <kernel/modules.h>
#include <modules/pit/pit.h>

static void pit_handle_irq(struct modules_device *self)
{

    struct pit_device *device = (struct pit_device *)self;

    device->jiffies += 1;

    event_raise(EVENT_IRQ_PIT);

}

void pit_device_init(struct pit_device *device)
{

    modules_device_init(&device->base, PIT_DEVICE_TYPE);

    device->divisor = PIT_HERTZ / PIT_FREQUENCY;
    device->jiffies = 0;

    io_outb(0x43, 0x36);
    io_outb(0x40, (unsigned char)(device->divisor & 0xFF));
    io_outb(0x40, (unsigned char)((device->divisor >> 8) & 0xFF));

    irq_register_routine(0x00, &device->base, pit_handle_irq);

}

