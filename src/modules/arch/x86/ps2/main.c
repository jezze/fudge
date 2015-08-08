#include <fudge.h>
#include <kernel.h>
#include <modules/base/base.h>
#include <modules/system/system.h>
#include <modules/arch/x86/io/io.h>
#include "ps2.h"

#define REGISTERDATA                    0x0060
#define REGISTERCONTROL                 0x0064
#define IRQKEYBOARD                     0x01
#define IRQMOUSE                        0x0C
#define COMMANDCONFIGR                  0x20
#define COMMANDCONFIGW                  0x60
#define COMMANDDEV2DISABLE              0xA7
#define COMMANDDEV2ENABLE               0xA8
#define COMMANDDEV2TEST                 0xA9
#define COMMANDCTEST                    0xAA
#define COMMANDDEV1TEST                 0xAB
#define COMMANDDEV1DISABLE              0xAD
#define COMMANDDEV1ENABLE               0xAE
#define COMMANDCTRLRO                   0xD0
#define COMMANDCTRLWO                   0xD1
#define COMMANDDEV1WO                   0xD2
#define COMMANDDEV2WO                   0xD3
#define COMMANDDEV2WI                   0xD4
#define COMMANDCTRLRESET                0xFE
#define COMMANDDEVIDENTIFY              0xF2
#define COMMANDDEVENABLESCAN            0xF4
#define COMMANDDEVDISABLESCAN           0xF5
#define COMMANDDEVDEFAULT               0xF6
#define COMMANDDEVRESET                 0xFF
#define STATUSOFULL                     (1 << 0)
#define STATUSIFULL                     (1 << 1)
#define STATUSSYSTEM                    (1 << 2)
#define STATUSCTRL                      (1 << 3)
#define STATUSTIMEOUT                   (1 << 6)
#define STATUSPARITY                    (1 << 7)
#define CONFIGFLAG_DEV1INT              (1 << 0)
#define CONFIGFLAG_DEV2INT              (1 << 1)
#define CONFIGFLAG_SYSTEM               (1 << 2)
#define CONFIGFLAG_DEV1CLOCK            (1 << 4)
#define CONFIGFLAG_DEV2CLOCK            (1 << 5)
#define CONFIGFLAG_DEV2TRANS            (1 << 6)
#define CTESTOK                         0x55
#define CTESTERROR                      0xFC
#define PTESTOK                         0x00
#define PTESTCLOCKLOW                   0x01
#define PTESTCLOCKHIGH                  0x02
#define PTESTLINELOW                    0x03
#define PTESTLINEHIGH                   0x04

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
    {0},
    {0, IRQKEYBOARD, COMMANDDEV1DISABLE, COMMANDDEV1ENABLE, COMMANDDEV1TEST, 0x00, CONFIGFLAG_DEV1INT, CONFIGFLAG_DEV1CLOCK},
    {0, IRQMOUSE, COMMANDDEV2DISABLE, COMMANDDEV2ENABLE, COMMANDDEV2TEST, COMMANDDEV2WI, CONFIGFLAG_DEV2INT, CONFIGFLAG_DEV2CLOCK}
};

static unsigned char getstatus(void)
{

    return io_inb(REGISTERCONTROL);

}

static unsigned char polldata(void)
{

    while ((getstatus() & STATUSOFULL) != 1);

    return io_inb(REGISTERDATA);

}

unsigned char ps2_getdata(void)
{

    return io_inb(REGISTERDATA);

}

unsigned short ps2_getirq(unsigned int id)
{

    return devices[id].irq;

}

void ps2_setcommand(unsigned char value)
{

    while ((getstatus() & STATUSIFULL) != 0);

    io_outb(REGISTERCONTROL, value);

}

void ps2_setdata(unsigned char value)
{

    while ((getstatus() & STATUSIFULL) != 0);

    io_outb(REGISTERDATA, value);

}

void ps2_enable(unsigned int id)
{

    ps2_setcommand(devices[id].enable);

}

void ps2_enableinterrupt(unsigned int id)
{

    unsigned char config;

    ps2_setcommand(COMMANDCONFIGR);

    config = polldata() | devices[id].interrupt;

    ps2_setcommand(COMMANDCONFIGW);
    ps2_setdata(config);

}

void ps2_reset(unsigned int id)
{

    if (devices[id].write)
        ps2_setcommand(devices[id].write);

    ps2_setdata(COMMANDDEVRESET);
    polldata();

}

void ps2_identify(unsigned int id)
{

    if (devices[id].write)
        ps2_setcommand(devices[id].write);

    ps2_setdata(COMMANDDEVIDENTIFY);
    polldata();
    polldata();

}

void ps2_enablescanning(unsigned int id)
{

    if (devices[id].write)
        ps2_setcommand(devices[id].write);

    ps2_setdata(COMMANDDEVENABLESCAN);
    polldata();

}

void ps2_disablescanning(unsigned int id)
{

    if (devices[id].write)
        ps2_setcommand(devices[id].write);

    ps2_setdata(COMMANDDEVDISABLESCAN);
    polldata();

}

void ps2_default(unsigned int id)
{

    if (devices[id].write)
        ps2_setcommand(devices[id].write);

    ps2_setdata(COMMANDDEVDEFAULT);
    polldata();

}

static void bus_setup(void)
{

    unsigned char config;
    unsigned char status;

    ps2_setcommand(devices[PS2_KEYBOARD].disable);
    ps2_setcommand(devices[PS2_MOUSE].disable);

    while (getstatus() & 1)
        ps2_getdata();

    ps2_setcommand(COMMANDCONFIGR);

    config = polldata() & 0xDC;

    ps2_setcommand(COMMANDCONFIGW);
    ps2_setdata(config);
    ps2_setcommand(COMMANDCTEST);

    status = polldata();

    if (status != CTESTOK)
        return;

    if (config & devices[PS2_KEYBOARD].clock)
    {

        ps2_setcommand(devices[PS2_KEYBOARD].test);

        if (polldata() == PTESTOK)
            devices[PS2_KEYBOARD].present = 1;

    }

    if (config & devices[PS2_MOUSE].clock)
    {

        ps2_setcommand(devices[PS2_MOUSE].test);

        if (polldata() == PTESTOK)
            devices[PS2_MOUSE].present = 1;

    }

}

static unsigned int bus_next(unsigned int id)
{

    return (id + 1) % 3;

}

static unsigned int reset_write(struct system_node *self, unsigned int offset, unsigned int size, unsigned int count, void *buffer)
{

    ps2_setcommand(COMMANDCTRLRESET);

    return 0;

}

void module_init(void)
{

    base_initbus(&bus, PS2_BUS, "ps2", bus_setup, bus_next);
    system_initnode(&reset, SYSTEM_NODETYPE_NORMAL, "reset");

    reset.write = reset_write;

}

void module_register(void)
{

    base_registerbus(&bus);
    system_registernode(&reset);

}

void module_unregister(void)
{

    base_unregisterbus(&bus);
    system_unregisternode(&reset);

}

