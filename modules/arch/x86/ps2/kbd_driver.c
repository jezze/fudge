#include <fudge/module.h>
#include <fudge/define.h>
#include <fudge/data/circular.h>
#include <system/system.h>
#include <base/base.h>
#include <kbd/kbd.h>
#include <mouse/mouse.h>
#include <arch/x86/pic/pic.h>
#include <arch/x86/io/io.h>
#include "ps2.h"

static void handle_irq(struct base_device *self)
{

    struct ps2_kbd_driver *driver = (struct ps2_kbd_driver *)self->driver;
    unsigned char scancode = io_inb(PS2_DATA);

    if (driver->escaped)
        driver->escaped = 0;

    if (scancode == 0xE0)
        driver->escaped = 1;

    circular_stream_write(&driver->stream, 1, &scancode);

}

static void attach(struct base_device *device)
{

    struct ps2_device *ps2device = (struct ps2_device *)device;

    pic_set_routine(ps2device->irq, device, handle_irq);

}

static unsigned int check(struct base_device *device)
{

    struct ps2_device *ps2device = (struct ps2_device *)device;

    if (device->type != PS2_DEVICE_TYPE)
        return 0;

    return ps2device->irq == PS2_IRQ_KEYBOARD;

}

static unsigned int read(struct kbd_interface *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct ps2_kbd_driver *driver = (struct ps2_kbd_driver *)self->driver;

    return circular_stream_read(&driver->stream, count, buffer);

}

static unsigned int write(struct kbd_interface *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct ps2_kbd_driver *driver = (struct ps2_kbd_driver *)self->driver;

    return circular_stream_write(&driver->stream, count, buffer);

}

void ps2_init_kbd_driver(struct ps2_kbd_driver *driver)
{

    memory_clear(driver, sizeof (struct ps2_kbd_driver));

    base_init_driver(&driver->base, PS2_KBD_DRIVER_TYPE, "ps2kbd", 0, check, attach);
    kbd_init_interface(&driver->interface, &driver->base, read, write);

}

