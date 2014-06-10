#include <module.h>
#include <kernel/resource.h>
#include <kernel/rendezvous.h>
#include <system/system.h>
#include <base/base.h>
#include <base/keyboard.h>
#include <base/mouse.h>
#include <arch/x86/io/io.h>
#include "ps2.h"
#include "keyboard_driver.h"
#include "mouse_driver.h"

#define PS2_KEYBOARD_IRQ                0x01
#define PS2_MOUSE_IRQ                   0x0C

enum ps2_register
{

    PS2_REGISTER_DATA                   = 0x0060,
    PS2_REGISTER_CONTROL                = 0x0064

};

static struct ps2_bus bus;
static struct base_device keyboard;
static struct base_device mouse;
static struct system_stream reset;

unsigned char ps2_bus_read_status(struct ps2_bus *bus)
{

    return io_inb(bus->control);

}

unsigned char ps2_bus_read_data(struct ps2_bus *bus)
{

    while ((ps2_bus_read_status(bus) & 1) != 1);

    return io_inb(bus->data);

}

unsigned char ps2_bus_read_data_async(struct ps2_bus *bus)
{

    return io_inb(bus->data);

}

void ps2_bus_write_command(struct ps2_bus *bus, unsigned char value)
{

    while ((ps2_bus_read_status(bus) & 2) != 0);

    io_outb(bus->control, value);

}

void ps2_bus_write_data(struct ps2_bus *bus, unsigned char value)
{

    while ((ps2_bus_read_status(bus) & 2) != 0);

    io_outb(bus->data, value);

}

void ps2_bus_reset(struct ps2_bus *bus)
{

    ps2_bus_write_command(bus, 0xFE);

}

void ps2_bus_enable_device(struct ps2_bus *bus, unsigned int type)
{

    unsigned int command = (type == PS2_MOUSE_DEVICE_TYPE) ? 0xA8 : 0xAE;

    ps2_bus_write_command(bus, command);

}

void ps2_bus_enable_interrupt(struct ps2_bus *bus, unsigned int type)
{

    unsigned char flag = (type == PS2_MOUSE_DEVICE_TYPE) ? 2 : 1;
    unsigned char status;

    ps2_bus_write_command(bus, 0x20);

    status = ps2_bus_read_data(bus) | flag;

    ps2_bus_write_command(bus, 0x60);
    ps2_bus_write_data(bus, status);

}

static void scan(struct base_bus *self)
{

    struct ps2_bus *bus = (struct ps2_bus *)self;
    unsigned char config;
    unsigned char status;

    ps2_bus_write_command(bus, 0xAD);
    ps2_bus_write_command(bus, 0xA7);

    while (ps2_bus_read_status(bus) & 1)
        ps2_bus_read_data_async(bus);

    ps2_bus_write_command(bus, 0x20);

    config = ps2_bus_read_data(bus);

    ps2_bus_write_command(bus, 0x60);
    ps2_bus_write_data(bus, config & 0xDC);
    ps2_bus_write_command(bus, 0xAA);

    status = ps2_bus_read_data(bus);

    if (status != 0x55)
        return;

    if (config & (1 << 4))
    {

        ps2_bus_write_command(bus, 0xAB);

        if (!ps2_bus_read_data(bus))
        {

            base_init_device(&keyboard, PS2_KEYBOARD_DEVICE_TYPE, "ps2", self);
            base_register_device(&keyboard);

        }

    }

    if (config & (1 << 5))
    {

        ps2_bus_write_command(bus, 0xA9);

        if (!ps2_bus_read_data(bus))
        {

            base_init_device(&mouse, PS2_MOUSE_DEVICE_TYPE, "ps2", self);
            base_register_device(&mouse);


        }

    }

}

static unsigned short device_irq(struct base_bus *self, struct base_device *device)
{

    switch (device->type)
    {

        case PS2_KEYBOARD_DEVICE_TYPE:
            return PS2_KEYBOARD_IRQ;

        case PS2_MOUSE_DEVICE_TYPE:
            return PS2_MOUSE_IRQ;

    }

    return 0xFFFF;

}

static unsigned int reset_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    ps2_bus_reset(&bus);

    return 0;

}

void init()
{

    base_init_bus(&bus.base, "ps2", scan, device_irq);

    bus.control = PS2_REGISTER_CONTROL;
    bus.data = PS2_REGISTER_DATA;

    base_register_bus(&bus.base);
    ps2_keyboard_driver_init();
    ps2_mouse_driver_init();
    system_init_stream(&reset, "reset");
    system_register_node(&reset.node);

    reset.node.write = reset_write;

}

void destroy()
{

    system_unregister_node(&reset.node);
    ps2_keyboard_driver_destroy();
    ps2_mouse_driver_destroy();
    base_unregister_bus(&bus.base);

}

