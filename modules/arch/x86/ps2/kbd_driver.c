#include <fudge/define.h>
#include <fudge/memory.h>
#include <fudge/data/circular.h>
#include <base/base.h>
#include <arch/x86/pic/pic.h>
#include <arch/x86/io/io.h>
#include "ps2.h"

static void handle_irq(struct base_device *self)
{

    unsigned char scancode;
    struct ps2_kbd_driver *kbd = (struct ps2_kbd_driver *)self->driver;

    if (!kbd)
        return;

    scancode = io_inb(PS2_DATA);

    if (kbd->escaped)
        kbd->escaped = 0;

    if (scancode == 0xE0)
        kbd->escaped = 1;

    circular_stream_write(&kbd->stream, 1, &scancode);

}

static void attach(struct base_device *device)
{

    struct ps2_device *ps2device = (struct ps2_device *)device;

    pic_set_routine(ps2device->irq, device, handle_irq);

}

static unsigned int check(struct base_driver *self, struct base_device *device)
{

    struct ps2_device *ps2device = (struct ps2_device *)device;

    if (device->type != PS2_DEVICE_TYPE)
        return 0;

    return ps2device->irq == PS2_IRQ_KEYBOARD;

}

void ps2_init_kbd_driver(struct ps2_kbd_driver *driver)
{

    memory_clear(driver, sizeof (struct ps2_kbd_driver));

    base_init_driver(&driver->base, PS2_KBD_DRIVER_TYPE, "kbd", 0, check, attach);

}

