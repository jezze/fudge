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

static void handle_irq(struct base_device *device)
{

    struct ps2_device *ps2device = (struct ps2_device *)device;
    struct ps2_kbd_driver *driver = (struct ps2_kbd_driver *)device->driver;
    unsigned char data = ps2device->bus->read_data_async();

    if (driver->escaped)
        driver->escaped = 0;

    if (data == 0xE0)
        driver->escaped = 1;

    circular_stream_write(&driver->stream, 1, &data);

}

static void attach(struct base_device *device)
{

    struct ps2_device *ps2device = (struct ps2_device *)device;
    unsigned char status;

    pic_set_routine(device->irq, device, handle_irq);
    ps2device->bus->write_command(0xAE);
    ps2device->bus->write_command(0x20);

    status = ps2device->bus->read_data() | 1;

    ps2device->bus->write_command(0x60);
    ps2device->bus->write_data(status);

}

static unsigned int check(struct base_device *device)
{

    if (device->type != PS2_DEVICE_TYPE)
        return 0;

    return device->irq == PS2_IRQ_KEYBOARD;

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
    base_init_driver(&driver->base, "ps2kbd", 0, check, attach);
    kbd_init_interface(&driver->interface, &driver->base, read, write);

}

