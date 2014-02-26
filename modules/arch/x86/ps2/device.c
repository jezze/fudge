#include <module.h>
#include <base/base.h>
#include "ps2.h"

void ps2_device_enable(struct ps2_device *device)
{

    struct ps2_bus *bus = (struct ps2_bus *)device->base.bus;
    unsigned int command = (device->base.irq == PS2_IRQ_MOUSE) ? 0xA8 : 0xAE;

    ps2_bus_write_command(bus, command);

}

void ps2_device_enable_interrupt(struct ps2_device *device)
{

    struct ps2_bus *bus = (struct ps2_bus *)device->base.bus;
    unsigned char flag = (device->base.irq == PS2_IRQ_MOUSE) ? 2 : 1;
    unsigned char status;

    ps2_bus_write_command(bus, 0x20);

    status = ps2_bus_read_data(bus) | flag;

    ps2_bus_write_command(bus, 0x60);
    ps2_bus_write_data(bus, status);

}

void ps2_init_device(struct ps2_device *device, struct ps2_bus *bus, unsigned int irq)
{

    memory_clear(device, sizeof (struct ps2_device));
    base_init_device(&device->base, PS2_DEVICE_TYPE, irq, "ps2", &bus->base);

}

