#include <module.h>
#include <kernel/resource.h>
#include <base/base.h>
#include "ps2.h"

void ps2_device_enable(struct base_device *device)
{

    struct ps2_bus *bus = (struct ps2_bus *)device->bus;
    unsigned int command = (device->type == PS2_MOUSE_DEVICE_TYPE) ? 0xA8 : 0xAE;

    ps2_bus_write_command(bus, command);

}

void ps2_device_enable_interrupt(struct base_device *device)
{

    struct ps2_bus *bus = (struct ps2_bus *)device->bus;
    unsigned char flag = (device->type == PS2_MOUSE_DEVICE_TYPE) ? 2 : 1;
    unsigned char status;

    ps2_bus_write_command(bus, 0x20);

    status = ps2_bus_read_data(bus) | flag;

    ps2_bus_write_command(bus, 0x60);
    ps2_bus_write_data(bus, status);

}

