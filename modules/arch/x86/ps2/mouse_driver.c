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
    struct ps2_mouse_driver *driver = (struct ps2_mouse_driver *)device->driver;

    switch (driver->cycle)
    {

        case 0:

            driver->status = ps2device->bus->read_data();
            driver->cycle++;

            break;

        case 1:

            driver->interface.vx = ps2device->bus->read_data();
            driver->cycle++;

            break;

        case 2:

            driver->interface.vy = ps2device->bus->read_data();
            driver->cycle = 0;

            break;

    }

}

static void attach(struct base_device *device)
{

    struct ps2_device *ps2device = (struct ps2_device *)device;
    unsigned char status;

    pic_set_routine(ps2device->irq, device, handle_irq);
    ps2device->bus->write_command(0xA8);
    ps2device->bus->write_command(0x20);

    status = ps2device->bus->read_data() | 2;

    ps2device->bus->write_command(0x60);
    ps2device->bus->write_data(status);
    ps2device->bus->write_command(0xD4);
    ps2device->bus->write_data(0xF6);
    ps2device->bus->read_data();
    ps2device->bus->write_command(0xD4);
    ps2device->bus->write_data(0xF4);
    ps2device->bus->read_data();

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
    base_init_driver(&driver->base, "ps2mouse", 0, check, attach);
    mouse_init_interface(&driver->interface, &driver->base);

    driver->cycle = 2;

}

