#include <kernel/arch/x86/io.h>
#include <kernel/event.h>
#include <kernel/irq.h>
#include <kernel/modules.h>
#include <modules/ps2/ps2.h>

static void handle_irq(struct modules_device *self)
{

    struct kbd_driver *kbd = (struct kbd_driver *)self->driver;

    if (!kbd)
        return;

    unsigned char scancode = io_inb(KBD_PORT_READ);

    if (kbd->escaped)
    {

        //scancode += 256;
        kbd->escaped = 0;

    }

    if (scancode == 0xE0)
        kbd->escaped = 1;

    if (scancode == 0x38)
        kbd->toggleAlt = 1;

    if (scancode == 0xB8)
        kbd->toggleAlt = 0;

    if (scancode == 0x1D)
        kbd->toggleCtrl = 1;

    if (scancode == 0x9D)
        kbd->toggleCtrl = 0;

    if (scancode == 0x2A)
        kbd->toggleShift = 1;

    if (scancode == 0xAA)
        kbd->toggleShift = 0;

    if (scancode & 0x80)
    {

        // Break codes

    }

    else
    {

        if (kbd->toggleShift)
            scancode += 128;

        kbd->buffer.putc(&kbd->buffer, &kbd->map[scancode]);

        event_raise(EVENT_IRQ_KBD);

    }

}

void kbd_device_init(struct kbd_device *device)
{

    modules_device_init(&device->base, KBD_DEVICE_TYPE, "kbd");

    irq_register_routine(0x01, &device->base, handle_irq);

}

