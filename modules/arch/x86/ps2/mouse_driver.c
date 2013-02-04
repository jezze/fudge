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

static void wait_read()
{

    while ((io_inb(PS2_COMMAND) & 1) != 1);

}

static void wait_write()
{

    while ((io_inb(PS2_COMMAND) & 2) != 0);

}

static void write(unsigned char value)
{

    wait_write();
    io_outb(PS2_COMMAND, 0xD4);
    wait_write();
    io_outb(PS2_DATA, value);

}

static unsigned char read()
{

    wait_read();

    return io_inb(PS2_DATA);

}

static void handle_irq(struct base_device *self)
{

    struct ps2_mouse_driver *driver = (struct ps2_mouse_driver *)self->driver;

    switch (driver->cycle)
    {

        case 0:

            driver->status = io_inb(0x60);
            driver->cycle++;

            break;

        case 1:

            driver->interface.vx = io_inb(0x60);
            driver->cycle++;

            break;

        case 2:

            driver->interface.vy = io_inb(0x60);
            driver->cycle = 0;

            break;

    }

}

static void start(struct base_driver *self)
{

    unsigned int status;

    wait_write();
    io_outb(PS2_COMMAND, 0xA8);
    wait_write();
    io_outb(PS2_COMMAND, 0x20);
    wait_read();

    status = (io_inb(PS2_DATA) | 2);

    wait_write();
    io_outb(PS2_COMMAND, 0x60);
    wait_write();
    io_outb(PS2_DATA, status);
    write(0xF6);
    read();
    write(0xF4);
    read();

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

    return ps2device->irq == PS2_IRQ_MOUSE;

}

void ps2_init_mouse_driver(struct ps2_mouse_driver *driver)
{

    memory_clear(driver, sizeof (struct ps2_mouse_driver));
    base_init_driver(&driver->base, "ps2mouse", start, check, attach);
    mouse_init_interface(&driver->interface, &driver->base);

    driver->cycle = 2;

}

