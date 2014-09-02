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

    PS2_COMMAND_CONFIGR                 = 0x20,
    PS2_COMMAND_CONFIGW                 = 0x60,
    PS2_COMMAND_DEV2DISABLE             = 0xA7,
    PS2_COMMAND_DEV2ENABLE              = 0xA8,
    PS2_COMMAND_DEV2TEST                = 0xA9,
    PS2_COMMAND_CTEST                   = 0xAA,
    PS2_COMMAND_DEV1TEST                = 0xAB,
    PS2_COMMAND_DEV1DISABLE             = 0xAD,
    PS2_COMMAND_DEV1ENABLE              = 0xAE,
    PS2_COMMAND_CTRLRO                  = 0xD0,
    PS2_COMMAND_CTRLWO                  = 0xD1,
    PS2_COMMAND_DEV1WO                  = 0xD2,
    PS2_COMMAND_DEV2WO                  = 0xD3,
    PS2_COMMAND_DEV2WI                  = 0xD4,
    PS2_COMMAND_CTRLRESET               = 0xFE,
    PS2_COMMAND_DEVIDENTIFY             = 0xF2,
    PS2_COMMAND_DEVENABLESCAN           = 0xF4,
    PS2_COMMAND_DEVDISABLESCAN          = 0xF5,
    PS2_COMMAND_DEVDEFAULT              = 0xF6,
    PS2_COMMAND_DEVRESET                = 0xFF

};

enum ps2_status
{

    PS2_STATUS_OFULL                    = (1 << 0),
    PS2_STATUS_IFULL                    = (1 << 1),
    PS2_STATUS_SYSTEM                   = (1 << 2),
    PS2_STATUS_CTRL                     = (1 << 3),
    PS2_STATUS_TIMEOUT                  = (1 << 6),
    PS2_STATUS_PARITY                   = (1 << 7)

};

enum ps2_configflag
{

    PS2_CONFIGFLAG_DEV1INT              = (1 << 0),
    PS2_CONFIGFLAG_DEV2INT              = (1 << 1),
    PS2_CONFIGFLAG_SYSTEM               = (1 << 2),
    PS2_CONFIGFLAG_DEV1CLOCK            = (1 << 4),
    PS2_CONFIGFLAG_DEV2CLOCK            = (1 << 5),
    PS2_CONFIGFLAG_DEV2TRANS            = (1 << 6)

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
    unsigned char write;
    unsigned char interrupt;
    unsigned char clock;

};

static struct base_bus bus;
static struct system_node reset;

static struct device devices[] = {
    {0, 0, 0, 0, 0, 0, 0},
    {0, PS2_IRQ_KEYBOARD, PS2_COMMAND_DEV1DISABLE, PS2_COMMAND_DEV1ENABLE, PS2_COMMAND_DEV1TEST, 0x00, PS2_CONFIGFLAG_DEV1INT, PS2_CONFIGFLAG_DEV1CLOCK},
    {0, PS2_IRQ_MOUSE, PS2_COMMAND_DEV2DISABLE, PS2_COMMAND_DEV2ENABLE, PS2_COMMAND_DEV2TEST, PS2_COMMAND_DEV2WI, PS2_CONFIGFLAG_DEV2INT, PS2_CONFIGFLAG_DEV2CLOCK}
};

static unsigned char getstatus(struct base_bus *bus)
{

    return io_inb(PS2_REGISTER_CONTROL);

}

static unsigned char polldata(struct base_bus *bus)
{

    while ((getstatus(bus) & PS2_STATUS_OFULL) != 1);

    return io_inb(PS2_REGISTER_DATA);

}

unsigned char ps2_getdata(struct base_bus *bus)
{

    return io_inb(PS2_REGISTER_DATA);

}

void ps2_setcommand(struct base_bus *bus, unsigned char value)
{

    while ((getstatus(bus) & PS2_STATUS_IFULL) != 0);

    io_outb(PS2_REGISTER_CONTROL, value);

}

void ps2_setdata(struct base_bus *bus, unsigned char value)
{

    while ((getstatus(bus) & PS2_STATUS_IFULL) != 0);

    io_outb(PS2_REGISTER_DATA, value);

}

void ps2_enable(struct base_bus *bus, unsigned int id)
{

    ps2_setcommand(bus, devices[id].enable);

}

void ps2_enableinterrupt(struct base_bus *bus, unsigned int id)
{

    unsigned char config;

    ps2_setcommand(bus, PS2_COMMAND_CONFIGR);

    config = polldata(bus) | devices[id].interrupt;

    ps2_setcommand(bus, PS2_COMMAND_CONFIGW);
    ps2_setdata(bus, config);

}

void ps2_reset(struct base_bus *bus, unsigned int id)
{

    if (devices[id].write)
        ps2_setcommand(bus, devices[id].write);

    ps2_setdata(bus, PS2_COMMAND_DEVRESET);
    polldata(bus);

}

void ps2_identify(struct base_bus *bus, unsigned int id)
{

    if (devices[id].write)
        ps2_setcommand(bus, devices[id].write);

    ps2_setdata(bus, PS2_COMMAND_DEVIDENTIFY);
    polldata(bus);
    polldata(bus);

}

void ps2_enablescanning(struct base_bus *bus, unsigned int id)
{

    if (devices[id].write)
        ps2_setcommand(bus, devices[id].write);

    ps2_setdata(bus, PS2_COMMAND_DEVENABLESCAN);
    polldata(bus);

}

void ps2_disablescanning(struct base_bus *bus, unsigned int id)
{

    if (devices[id].write)
        ps2_setcommand(bus, devices[id].write);

    ps2_setdata(bus, PS2_COMMAND_DEVDISABLESCAN);
    polldata(bus);

}

void ps2_default(struct base_bus *bus, unsigned int id)
{

    if (devices[id].write)
        ps2_setcommand(bus, devices[id].write);

    ps2_setdata(bus, PS2_COMMAND_DEVDEFAULT);
    polldata(bus);

}

static void bus_setup(struct base_bus *self)
{

    unsigned char config;
    unsigned char status;

    ps2_setcommand(self, devices[PS2_KEYBOARD_DEVICE_TYPE].disable);
    ps2_setcommand(self, devices[PS2_MOUSE_DEVICE_TYPE].disable);

    while (getstatus(self) & 1)
        ps2_getdata(self);

    ps2_setcommand(self, PS2_COMMAND_CONFIGR);

    config = polldata(self) & 0xDC;

    ps2_setcommand(self, PS2_COMMAND_CONFIGW);
    ps2_setdata(self, config);
    ps2_setcommand(self, PS2_COMMAND_CTEST);

    status = polldata(self);

    if (status != PS2_CTEST_OK)
        return;

    if (config & devices[PS2_KEYBOARD_DEVICE_TYPE].clock)
    {

        ps2_setcommand(self, devices[PS2_KEYBOARD_DEVICE_TYPE].test);

        if (polldata(self) == PS2_PTEST_OK)
            devices[PS2_KEYBOARD_DEVICE_TYPE].present = 1;

    }

    if (config & devices[PS2_MOUSE_DEVICE_TYPE].clock)
    {

        ps2_setcommand(self, devices[PS2_MOUSE_DEVICE_TYPE].test);

        if (polldata(self) == PS2_PTEST_OK)
            devices[PS2_MOUSE_DEVICE_TYPE].present = 1;

    }

}

static unsigned int bus_next(struct base_bus *self, unsigned int id)
{

    return (id < 2) ? id + 1 : 0;

}

static unsigned short bus_irq(struct base_bus *self, unsigned int id)
{

    return (id < 3) ? devices[id].irq : 0xFFFF;

}

static unsigned int reset_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    ps2_setcommand(&bus, PS2_COMMAND_CTRLRESET);

    return 0;

}

void init()
{

    base_initbus(&bus, PS2_BUS_TYPE, "ps2", bus_setup, bus_next, bus_irq);
    base_registerbus(&bus);
    ps2_keyboard_driver_init();
    ps2_mouse_driver_init();
    system_initstream(&reset, "reset");
    system_registernode(&reset);

    reset.write = reset_write;

}

void destroy()
{

    system_unregisternode(&reset);
    ps2_keyboard_driver_destroy();
    ps2_mouse_driver_destroy();
    base_unregisterbus(&bus);

}

