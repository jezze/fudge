#include <module.h>
#include <kernel/resource.h>
#include <base/base.h>
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

static void add_device(struct ps2_bus *bus, unsigned int type)
{

    struct base_device *device = &bus->devices.item[bus->devices.count];

    base_init_device(device, type, "ps2", &bus->base);

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
            add_device(bus, PS2_KEYBOARD_DEVICE_TYPE);

    }

    if (config & (1 << 5))
    {

        ps2_bus_write_command(bus, 0xA9);

        if (!ps2_bus_read_data(bus))
            add_device(bus, PS2_MOUSE_DEVICE_TYPE);

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

void ps2_init_bus(struct ps2_bus *bus, unsigned short control, unsigned short data)
{

    memory_clear(bus, sizeof (struct ps2_bus));
    base_init_bus(&bus->base, PS2_BUS_TYPE, "ps2", scan, device_irq);

    bus->control = control;
    bus->data = data;

}

