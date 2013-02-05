#include <fudge/module.h>
#include <fudge/define.h>
#include <fudge/data/circular.h>
#include <system/system.h>
#include <base/base.h>
#include <kbd/kbd.h>
#include <mouse/mouse.h>
#include <arch/x86/io/io.h>
#include "ps2.h"

unsigned char ps2_bus_read_status()
{

    return io_inb(PS2_COMMAND);

}

unsigned char ps2_bus_read_data()
{

    while ((ps2_bus_read_status() & 1) != 1);

    return io_inb(PS2_DATA);

}

unsigned char ps2_bus_read_data_async()
{

    return io_inb(PS2_DATA);

}

void ps2_bus_write_command(unsigned char value)
{

    while ((ps2_bus_read_status() & 2) != 0);

    io_outb(PS2_COMMAND, value);

}

void ps2_bus_write_data(unsigned char value)
{

    while ((ps2_bus_read_status() & 2) != 0);

    io_outb(PS2_DATA, value);

}

void ps2_bus_reset()
{

    ps2_bus_write_command(0xFE);

}

static void add_device(struct ps2_bus *bus, unsigned int irq)
{

    struct ps2_device *device = &bus->devices[bus->devicesCount];

    ps2_init_device(device, bus, "ps2", irq);
    base_register_device(&device->base);

    bus->devicesCount++;

}

static void scan(struct base_bus *self)
{

    struct ps2_bus *bus = (struct ps2_bus *)self;
    unsigned char config;
    unsigned char status;

    ps2_bus_write_command(0xAD);
    ps2_bus_write_command(0xA7);
    ps2_bus_write_command(0x20);

    config = ps2_bus_read_data();

    ps2_bus_write_command(0x60);
    ps2_bus_write_data(config & 0xDC);
    ps2_bus_write_command(0xAA);

    status = ps2_bus_read_data();

    if (status != 0x55)
        return;

    if (config & (1 << 4))
    {

        ps2_bus_write_command(0xAB);

        if (!ps2_bus_read_data())
            add_device(bus, PS2_IRQ_KEYBOARD);

    }

    if (config & (1 << 5))
    {

        ps2_bus_write_command(0xA9);

        if (!ps2_bus_read_data())
            add_device(bus, PS2_IRQ_MOUSE);

    }

}

void ps2_init_bus(struct ps2_bus *bus)
{

    memory_clear(bus, sizeof (struct ps2_bus));
    base_init_bus(&bus->base, PS2_BUS_TYPE, "ps2", scan);

}

