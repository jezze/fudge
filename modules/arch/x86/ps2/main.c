#include <module.h>
#include <kernel/resource.h>
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

enum ps2_command
{

    PS2_COMMAND_RCONFIG                 = 0x20,
    PS2_COMMAND_WCONFIG                 = 0x60,
    PS2_COMMAND_P2DISABLE               = 0xA7,
    PS2_COMMAND_P2ENABLE                = 0xA8,
    PS2_COMMAND_P2TEST                  = 0xA9,
    PS2_COMMAND_CTEST                   = 0xAA,
    PS2_COMMAND_P1TEST                  = 0xAB,
    PS2_COMMAND_P1DISABLE               = 0xAD,
    PS2_COMMAND_P1ENABLE                = 0xAE,
    PS2_COMMAND_RESET                   = 0xFE

};

enum ps2_status
{

    PS2_STATUS_OFULL                    = (1 << 0),
    PS2_STATUS_IFULL                    = (1 << 1),
    PS2_STATUS_SYSTEM                   = (1 << 2),
    PS2_STATUS_CONTROLLER               = (1 << 3),
    PS2_STATUS_TIMEOUT                  = (1 << 6),
    PS2_STATUS_PARITY                   = (1 << 7)

};

enum ps2_configflag
{

    PS2_CONFIGFLAG_P1INT                = (1 << 0),
    PS2_CONFIGFLAG_P2INT                = (1 << 1),
    PS2_CONFIGFLAG_SYSTEM               = (1 << 2),
    PS2_CONFIGFLAG_P1CLOCK              = (1 << 4),
    PS2_CONFIGFLAG_P2CLOCK              = (1 << 5),
    PS2_CONFIGFLAG_P2TRANS              = (1 << 6)

};

enum ps2_ctest
{

    PS2_CTEST_OK                        = 0x55,
    PS2_CTEST_ERROR                     = 0xFC

};

enum ps2_ptest
{

    PS2_PTEST_OK                        = 0x00,
    PS2_PTEST_CLOCKLOW                  = 0x01,
    PS2_PTEST_CLOCKHIGH                 = 0x02,
    PS2_PTEST_LINELOW                   = 0x03,
    PS2_PTEST_LINEHIGH                  = 0x04

};

struct device
{

    unsigned int present;
    unsigned short irq;
    unsigned char disable;
    unsigned char enable;
    unsigned char test;
    unsigned char interrupt;
    unsigned char clock;

};

static struct base_bus bus;
static struct system_node reset;

static struct device devices[] = {
    {0, 0, 0, 0, 0, 0, 0},
    {0, PS2_IRQ_KEYBOARD, PS2_COMMAND_P1DISABLE, PS2_COMMAND_P1ENABLE, PS2_COMMAND_P1TEST, PS2_CONFIGFLAG_P1INT, PS2_CONFIGFLAG_P1CLOCK},
    {0, PS2_IRQ_MOUSE, PS2_COMMAND_P2DISABLE, PS2_COMMAND_P2ENABLE, PS2_COMMAND_P2TEST, PS2_CONFIGFLAG_P2INT, PS2_CONFIGFLAG_P2CLOCK}
};

unsigned char ps2_bus_read_status(struct base_bus *bus)
{

    return io_inb(PS2_REGISTER_CONTROL);

}

unsigned char ps2_bus_read_data(struct base_bus *bus)
{

    while ((ps2_bus_read_status(bus) & PS2_STATUS_OFULL) != 1);

    return io_inb(PS2_REGISTER_DATA);

}

unsigned char ps2_bus_read_data_async(struct base_bus *bus)
{

    return io_inb(PS2_REGISTER_DATA);

}

void ps2_bus_write_command(struct base_bus *bus, unsigned char value)
{

    while ((ps2_bus_read_status(bus) & PS2_STATUS_IFULL) != 0);

    io_outb(PS2_REGISTER_CONTROL, value);

}

void ps2_bus_write_data(struct base_bus *bus, unsigned char value)
{

    while ((ps2_bus_read_status(bus) & PS2_STATUS_IFULL) != 0);

    io_outb(PS2_REGISTER_DATA, value);

}

void ps2_bus_reset(struct base_bus *bus)
{

    ps2_bus_write_command(bus, PS2_COMMAND_RESET);

}

void ps2_bus_enable_device(struct base_bus *bus, unsigned int id)
{

    ps2_bus_write_command(bus, devices[id].enable);

}

void ps2_bus_enable_interrupt(struct base_bus *bus, unsigned int id)
{

    unsigned char config;

    ps2_bus_write_command(bus, PS2_COMMAND_RCONFIG);

    config = ps2_bus_read_data(bus) | devices[id].interrupt;

    ps2_bus_write_command(bus, PS2_COMMAND_WCONFIG);
    ps2_bus_write_data(bus, config);

}

static void setup(struct base_bus *self)
{

    unsigned char config;
    unsigned char status;

    ps2_bus_write_command(self, devices[PS2_KEYBOARD_DEVICE_TYPE].disable);
    ps2_bus_write_command(self, devices[PS2_MOUSE_DEVICE_TYPE].disable);

    while (ps2_bus_read_status(self) & 1)
        ps2_bus_read_data_async(self);

    ps2_bus_write_command(self, PS2_COMMAND_RCONFIG);

    config = ps2_bus_read_data(self) & 0xDC;

    ps2_bus_write_command(self, PS2_COMMAND_WCONFIG);
    ps2_bus_write_data(self, config);
    ps2_bus_write_command(self, PS2_COMMAND_CTEST);

    status = ps2_bus_read_data(self);

    if (status != PS2_CTEST_OK)
        return;

    if (config & devices[PS2_KEYBOARD_DEVICE_TYPE].clock)
    {

        ps2_bus_write_command(self, devices[PS2_KEYBOARD_DEVICE_TYPE].test);

        if (ps2_bus_read_data(self) == PS2_PTEST_OK)
            devices[PS2_KEYBOARD_DEVICE_TYPE].present = 1;

    }

    if (config & devices[PS2_MOUSE_DEVICE_TYPE].clock)
    {

        ps2_bus_write_command(self, devices[PS2_MOUSE_DEVICE_TYPE].test);

        if (ps2_bus_read_data(self) == PS2_PTEST_OK)
            devices[PS2_MOUSE_DEVICE_TYPE].present = 1;

    }

}

static unsigned int device_next(struct base_bus *self, unsigned int id)
{

    return (id < 2) ? id + 1 : 0;

}

static unsigned short device_irq(struct base_bus *self, unsigned int id)
{

    return (id < 3) ? devices[id].irq : 0xFFFF;

}

static unsigned int reset_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    ps2_bus_reset(&bus);

    return 0;

}

void init()
{

    base_init_bus(&bus, PS2_BUS_TYPE, "ps2", setup, device_next, device_irq);
    base_register_bus(&bus);
    ps2_keyboard_driver_init();
    ps2_mouse_driver_init();
    system_init_stream(&reset, "reset");
    system_register_node(&reset);

    reset.write = reset_write;

}

void destroy()
{

    system_unregister_node(&reset);
    ps2_keyboard_driver_destroy();
    ps2_mouse_driver_destroy();
    base_unregister_bus(&bus);

}

