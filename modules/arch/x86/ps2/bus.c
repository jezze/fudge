#include <fudge/module.h>
#include <fudge/define.h>
#include <fudge/data/circular.h>
#include <base/base.h>
#include <kbd/kbd.h>
#include <mouse/mouse.h>
#include <arch/x86/io/io.h>
#include "ps2.h"

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

static void add_device(struct ps2_bus *bus, unsigned int irq)
{

    struct ps2_device *device = &bus->devices.item[bus->devices.count];

    ps2_init_device(device, bus, "ps2", irq);

    bus->devices.count++;

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
            add_device(bus, PS2_IRQ_KEYBOARD);

    }

    if (config & (1 << 5))
    {

        ps2_bus_write_command(bus, 0xA9);

        if (!ps2_bus_read_data(bus))
            add_device(bus, PS2_IRQ_MOUSE);

    }

}

void ps2_init_bus(struct ps2_bus *bus)
{

    memory_clear(bus, sizeof (struct ps2_bus));
    base_init_bus(&bus->base, PS2_BUS_TYPE, "ps2", scan);

    bus->control = PS2_BUS_CONTROL;
    bus->data = PS2_BUS_DATA;

}

