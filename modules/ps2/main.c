#include <kernel/arch/x86/io.h>
#include <kernel/event.h>
#include <kernel/modules.h>
#include <kernel/kernel.h>
#include <modules/ps2/ps2.h>

static struct kbd_device kbdDevice;
static struct mouse_device mouseDevice;

static void handle_kbd_irq()
{

    struct kbd_device *kbd = &kbdDevice;

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

    }

    event_handle(EVENT_IRQ_KBD);

}

static void handle_mouse_irq()
{

    struct mouse_device *mouse = &mouseDevice;

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

    event_handle(EVENT_IRQ_MOUSE);

}

void init()
{

    kbd_device_init(&kbdDevice);
    mouse_device_init(&mouseDevice);

    modules_register_device(&kbdDevice.base);
    modules_register_device(&mouseDevice.base);

    kernel_register_irq(0x01, handle_kbd_irq);
    kernel_register_irq(0x0C, handle_mouse_irq);

}

void destroy()
{

    kernel_unregister_irq(0x01);
    kernel_unregister_irq(0x0C);

    modules_unregister_device(&kbdDevice.base);
    modules_unregister_device(&mouseDevice.base);

}

