#include <fudge/module.h>
#include <fudge/define.h>
#include <fudge/data/circular.h>
#include <base/base.h>
#include <kbd/kbd.h>
#include <mouse/mouse.h>
#include <arch/x86/pic/pic.h>
#include <arch/x86/io/io.h>
#include "ps2.h"

static void handle_irq(struct base_device *device)
{

    struct ps2_device *ps2Device = (struct ps2_device *)device;
    struct ps2_mouse_driver *driver = (struct ps2_mouse_driver *)device->driver;
    unsigned char data = ps2_bus_read_data_async(ps2Device->bus);

    switch (driver->cycle)
    {

        case 0:

            driver->status = data;
            driver->cycle++;

            break;

        case 1:

            driver->imouse.vx = data;
            driver->cycle++;

            break;

        case 2:

            driver->imouse.vy = data;
            driver->cycle = 0;

            break;

    }

}

static void attach(struct base_device *device)
{

    struct ps2_device *ps2Device = (struct ps2_device *)device;
    unsigned char status;

    pic_set_routine(device->irq, device, handle_irq);
    ps2_bus_write_command(ps2Device->bus, 0xA8);
    ps2_bus_write_command(ps2Device->bus, 0x20);

    status = ps2_bus_read_data(ps2Device->bus) | 2;

    ps2_bus_write_command(ps2Device->bus, 0x60);
    ps2_bus_write_data(ps2Device->bus, status);
    ps2_bus_write_command(ps2Device->bus, 0xD4);
    ps2_bus_write_data(ps2Device->bus, 0xF6);
    ps2_bus_read_data(ps2Device->bus);
    ps2_bus_write_command(ps2Device->bus, 0xD4);
    ps2_bus_write_data(ps2Device->bus, 0xF4);
    ps2_bus_read_data(ps2Device->bus);

}

static unsigned int check(struct base_device *device)
{

    if (device->type != PS2_DEVICE_TYPE)
        return 0;

    return device->irq == PS2_IRQ_MOUSE;

}

void ps2_init_mouse_driver(struct ps2_mouse_driver *driver)
{

    memory_clear(driver, sizeof (struct ps2_mouse_driver));
    base_init_driver(&driver->base, "ps2", 0, check, attach);
    mouse_init_interface(&driver->imouse, &driver->base);

    driver->cycle = 2;

}

