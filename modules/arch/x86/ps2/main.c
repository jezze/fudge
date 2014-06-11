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

enum ps2_register
{

    PS2_REGISTER_DATA                   = 0x0060,
    PS2_REGISTER_CONTROL                = 0x0064

};

enum ps2_irq
{

    PS2_IRQ_KEYBOARD                    = 0x01,
    PS2_IRQ_MOUSE                       = 0x0C

};

static struct base_bus bus;
static struct base_device keyboard;
static struct base_device mouse;
static struct system_stream reset;

unsigned char ps2_bus_read_status(struct base_bus *bus)
{

    return io_inb(PS2_REGISTER_CONTROL);

}

unsigned char ps2_bus_read_data(struct base_bus *bus)
{

    while ((ps2_bus_read_status(bus) & 1) != 1);

    return io_inb(PS2_REGISTER_DATA);

}

unsigned char ps2_bus_read_data_async(struct base_bus *bus)
{

    return io_inb(PS2_REGISTER_DATA);

}

void ps2_bus_write_command(struct base_bus *bus, unsigned char value)
{

    while ((ps2_bus_read_status(bus) & 2) != 0);

    io_outb(PS2_REGISTER_CONTROL, value);

}

void ps2_bus_write_data(struct base_bus *bus, unsigned char value)
{

    while ((ps2_bus_read_status(bus) & 2) != 0);

    io_outb(PS2_REGISTER_DATA, value);

}

void ps2_bus_reset(struct base_bus *bus)
{

    ps2_bus_write_command(bus, 0xFE);

}

void ps2_bus_enable_device(struct base_bus *bus, unsigned int type)
{

    unsigned int command = (type == PS2_MOUSE_DEVICE_TYPE) ? 0xA8 : 0xAE;

    ps2_bus_write_command(bus, command);

}

void ps2_bus_enable_interrupt(struct base_bus *bus, unsigned int type)
{

    unsigned char flag = (type == PS2_MOUSE_DEVICE_TYPE) ? 2 : 1;
    unsigned char status;

    ps2_bus_write_command(bus, 0x20);

    status = ps2_bus_read_data(bus) | flag;

    ps2_bus_write_command(bus, 0x60);
    ps2_bus_write_data(bus, status);

}

static void setup(struct base_bus *self)
{

    unsigned char config;
    unsigned char status;

    ps2_bus_write_command(self, 0xAD);
    ps2_bus_write_command(self, 0xA7);

    while (ps2_bus_read_status(self) & 1)
        ps2_bus_read_data_async(self);

    ps2_bus_write_command(self, 0x20);

    config = ps2_bus_read_data(self);

    ps2_bus_write_command(self, 0x60);
    ps2_bus_write_data(self, config & 0xDC);
    ps2_bus_write_command(self, 0xAA);

    status = ps2_bus_read_data(self);

    if (status != 0x55)
        return;

    if (config & (1 << 4))
    {

        ps2_bus_write_command(self, 0xAB);

        if (!ps2_bus_read_data(self))
        {

            base_init_device(&keyboard, PS2_KEYBOARD_DEVICE_TYPE, "ps2", self);
            base_register_device(&keyboard);

        }

    }

    if (config & (1 << 5))
    {

        ps2_bus_write_command(self, 0xA9);

        if (!ps2_bus_read_data(self))
        {

            base_init_device(&mouse, PS2_MOUSE_DEVICE_TYPE, "ps2", self);
            base_register_device(&mouse);

        }

    }

}

static unsigned short device_irq(struct base_bus *self, unsigned int id)
{

    switch (id)
    {

        case PS2_KEYBOARD_DEVICE_TYPE:
            return PS2_IRQ_KEYBOARD;

        case PS2_MOUSE_DEVICE_TYPE:
            return PS2_IRQ_MOUSE;

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

    base_init_bus(&bus, "ps2", setup, device_irq);
    base_register_bus(&bus);
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
    base_unregister_bus(&bus);

}

