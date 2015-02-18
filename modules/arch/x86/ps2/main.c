#include <fudge.h>
#include <kernel.h>
#include <base/base.h>
#include <system/system.h>
#include <arch/x86/io/io.h>
#include "ps2.h"

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

static unsigned char getstatus()
{

    return io_inb(PS2_REGISTER_CONTROL);

}

static unsigned char polldata()
{

    while ((getstatus() & PS2_STATUS_OFULL) != 1);

    return io_inb(PS2_REGISTER_DATA);

}

unsigned char ps2_getdata()
{

    return io_inb(PS2_REGISTER_DATA);

}

unsigned short ps2_getirq(unsigned int id)
{

    return (id < 3) ? devices[id].irq : 0xFFFF;

}

void ps2_setcommand(unsigned char value)
{

    while ((getstatus() & PS2_STATUS_IFULL) != 0);

    io_outb(PS2_REGISTER_CONTROL, value);

}

void ps2_setdata(unsigned char value)
{

    while ((getstatus() & PS2_STATUS_IFULL) != 0);

    io_outb(PS2_REGISTER_DATA, value);

}

void ps2_enable(unsigned int id)
{

    ps2_setcommand(devices[id].enable);

}

void ps2_enableinterrupt(unsigned int id)
{

    unsigned char config;

    ps2_setcommand(PS2_COMMAND_CONFIGR);

    config = polldata() | devices[id].interrupt;

    ps2_setcommand(PS2_COMMAND_CONFIGW);
    ps2_setdata(config);

}

void ps2_reset(unsigned int id)
{

    if (devices[id].write)
        ps2_setcommand(devices[id].write);

    ps2_setdata(PS2_COMMAND_DEVRESET);
    polldata();

}

void ps2_identify(unsigned int id)
{

    if (devices[id].write)
        ps2_setcommand(devices[id].write);

    ps2_setdata(PS2_COMMAND_DEVIDENTIFY);
    polldata();
    polldata();

}

void ps2_enablescanning(unsigned int id)
{

    if (devices[id].write)
        ps2_setcommand(devices[id].write);

    ps2_setdata(PS2_COMMAND_DEVENABLESCAN);
    polldata();

}

void ps2_disablescanning(unsigned int id)
{

    if (devices[id].write)
        ps2_setcommand(devices[id].write);

    ps2_setdata(PS2_COMMAND_DEVDISABLESCAN);
    polldata();

}

void ps2_default(unsigned int id)
{

    if (devices[id].write)
        ps2_setcommand(devices[id].write);

    ps2_setdata(PS2_COMMAND_DEVDEFAULT);
    polldata();

}

static void bus_setup()
{

    unsigned char config;
    unsigned char status;

    ps2_setcommand(devices[PS2_KEYBOARD_DEVICE_TYPE].disable);
    ps2_setcommand(devices[PS2_MOUSE_DEVICE_TYPE].disable);

    while (getstatus() & 1)
        ps2_getdata();

    ps2_setcommand(PS2_COMMAND_CONFIGR);

    config = polldata() & 0xDC;

    ps2_setcommand(PS2_COMMAND_CONFIGW);
    ps2_setdata(config);
    ps2_setcommand(PS2_COMMAND_CTEST);

    status = polldata();

    if (status != PS2_CTEST_OK)
        return;

    if (config & devices[PS2_KEYBOARD_DEVICE_TYPE].clock)
    {

        ps2_setcommand(devices[PS2_KEYBOARD_DEVICE_TYPE].test);

        if (polldata() == PS2_PTEST_OK)
            devices[PS2_KEYBOARD_DEVICE_TYPE].present = 1;

    }

    if (config & devices[PS2_MOUSE_DEVICE_TYPE].clock)
    {

        ps2_setcommand(devices[PS2_MOUSE_DEVICE_TYPE].test);

        if (polldata() == PS2_PTEST_OK)
            devices[PS2_MOUSE_DEVICE_TYPE].present = 1;

    }

}

static unsigned int bus_next(unsigned int id)
{

    return (id < 2) ? id + 1 : 0;

}

static unsigned int reset_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    ps2_setcommand(PS2_COMMAND_CTRLRESET);

    return 0;

}

void module_init()
{

    base_initbus(&bus, PS2_BUS_TYPE, "ps2", bus_setup, bus_next);
    system_initnode(&reset, SYSTEM_NODETYPE_NORMAL, "reset");

    reset.write = reset_write;

}

void module_register()
{

    base_registerbus(&bus);
    system_registernode(&reset);

}

void module_unregister()
{

    base_unregisterbus(&bus);
    system_unregisternode(&reset);

}

