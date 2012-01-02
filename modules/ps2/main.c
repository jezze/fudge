#include <kernel/arch/x86/io.h>
#include <kernel/event.h>
#include <kernel/irq.h>
#include <kernel/modules.h>
#include <modules/ps2/ps2.h>

static struct kbd_driver kbdDriver;
static struct mouse_driver mouseDriver;

static void handle_kbd_irq(struct modules_device *self)
{

    struct kbd_driver *kbd = &kbdDriver;

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

static void handle_mouse_irq(struct modules_device *self)
{

    struct mouse_driver *mouse = &mouseDriver;

    switch (mouse->cycle)
    {

        case 0:

            mouse->byte[0] = io_inb(0x60);
            mouse->cycle++;

            break;

        case 1:

            mouse->byte[1] = io_inb(0x60);
            mouse->cycle++;

            break;

        case 2:

            mouse->byte[2] = io_inb(0x60);
            mouse->x = mouse->byte[1];
            mouse->y = mouse->byte[2];
            mouse->cycle = 0;

            break;

    }

    event_raise(EVENT_IRQ_MOUSE);

}

void init()
{

    kbd_driver_init(&kbdDriver);
    mouse_driver_init(&mouseDriver);

    modules_register_driver(&kbdDriver.base);
    modules_register_driver(&mouseDriver.base);

    irq_register_routine(0x01, 0, handle_kbd_irq);
    irq_register_routine(0x0C, 0, handle_mouse_irq);

}

void destroy()
{

    irq_unregister_routine(0x01, 0);
    irq_unregister_routine(0x0C, 0);

    modules_unregister_driver(&kbdDriver.base);
    modules_unregister_driver(&mouseDriver.base);

}

