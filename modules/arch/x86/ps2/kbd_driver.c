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

    struct ps2_bus *bus = (struct ps2_bus *)device->bus;
    struct ps2_kbd_driver *driver = (struct ps2_kbd_driver *)device->driver;
    unsigned char scancode = ps2_bus_read_data_async(bus);

    if (driver->escaped)
        driver->escaped = 0;

    if (scancode == 0xE0)
        driver->escaped = 1;

    if (scancode & 0x80)
    {

        scancode &= ~0x80;

        if (scancode == 0x1D)
            driver->ctrl = 0;

        if (scancode == 0x2A)
            driver->shift = 0;

        if (scancode == 0x38)
            driver->alt = 0;

    }

    else
    {

        if (scancode == 0x1D)
            driver->ctrl = 1;

        if (scancode == 0x2A)
            driver->shift = 1;

        if (scancode == 0x38)
            driver->alt = 1;

        if (driver->ctrl)
            scancode = 0;

        if (driver->alt)
            scancode = 0;

        if (driver->shift)
            scancode += 128;

        circular_stream_write(&driver->stream, 1, driver->ikbd.keymap + scancode);

    }

}

static void attach(struct base_device *device)
{

    struct ps2_bus *bus = (struct ps2_bus *)device->bus;
    struct ps2_kbd_driver *driver = (struct ps2_kbd_driver *)device->driver;
    unsigned char status;

    kbd_register_device(&driver->ikbd, device);
    pic_set_routine(device, handle_irq);
    ps2_bus_write_command(bus, 0xAE);
    ps2_bus_write_command(bus, 0x20);

    status = ps2_bus_read_data(bus) | 1;

    ps2_bus_write_command(bus, 0x60);
    ps2_bus_write_data(bus, status);

}

static unsigned int check(struct base_device *device)
{

    if (device->type != PS2_DEVICE_TYPE)
        return 0;

    return device->irq == PS2_IRQ_KEYBOARD;

}

static unsigned int read_data(struct base_device *device, unsigned int offset, unsigned int count, void *buffer)
{

    struct ps2_kbd_driver *driver = (struct ps2_kbd_driver *)device->driver;

    return circular_stream_read(&driver->stream, count, buffer);

}

static unsigned int write_data(struct base_device *device, unsigned int offset, unsigned int count, void *buffer)
{

    struct ps2_kbd_driver *driver = (struct ps2_kbd_driver *)device->driver;

    return circular_stream_write(&driver->stream, count, buffer);

}

void ps2_init_kbd_driver(struct ps2_kbd_driver *driver)
{

    memory_clear(driver, sizeof (struct ps2_kbd_driver));
    base_init_driver(&driver->base, "ps2", check, attach);
    kbd_init_interface(&driver->ikbd, read_data, write_data);

}

