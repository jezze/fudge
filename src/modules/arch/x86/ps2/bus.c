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
#define COMMANDDEVRATE                  0xF3
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
#define CONFIGFLAG_DEV1TRANS            (1 << 6)
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
    unsigned char interrupt;
    unsigned char translation;
    unsigned char test;

};

static struct base_bus bus;
static struct system_node reset;

static struct device devices[] = {
    {0},
    {0, IRQKEYBOARD, COMMANDDEV1DISABLE, COMMANDDEV1ENABLE, CONFIGFLAG_DEV1INT, CONFIGFLAG_DEV1TRANS, COMMANDDEV1TEST},
    {0, IRQMOUSE, COMMANDDEV2DISABLE, COMMANDDEV2ENABLE, CONFIGFLAG_DEV2INT, 0, COMMANDDEV2TEST}
};

static void flushdata(void)
{

    while ((io_inb(REGISTERCONTROL) & STATUSOFULL))
        io_inb(REGISTERDATA);

}

static unsigned char polldata(void)
{

    while (!(io_inb(REGISTERCONTROL) & STATUSOFULL));

    return io_inb(REGISTERDATA);

}

static void setcommand(unsigned char value)
{

    while ((io_inb(REGISTERCONTROL) & STATUSIFULL));

    io_outb(REGISTERCONTROL, value);

}

static void setdata(unsigned char value)
{

    while ((io_inb(REGISTERCONTROL) & STATUSIFULL));

    io_outb(REGISTERDATA, value);

}

static void setdevicedata(unsigned int id, unsigned char value)
{

    if (id == PS2_MOUSE)
        setcommand(COMMANDDEV2WI);

    setdata(value);

}

static unsigned char rconfig(void)
{

    setcommand(COMMANDCONFIGR);

    return polldata();

}

static void wconfig(unsigned char config)
{

    setcommand(COMMANDCONFIGW);
    setdata(config);

}

static unsigned int testbus(void)
{

    setcommand(COMMANDCTEST);

    return polldata() == CTESTOK;

}

static unsigned int testdevice(unsigned int id)
{

    setcommand(devices[id].test);

    return polldata() == PTESTOK;

}

unsigned int ps2_checkdata(unsigned int id)
{

    unsigned char control = io_inb(REGISTERCONTROL);

    if ((control & STATUSTIMEOUT))
        return 0;

    if ((control & STATUSPARITY))
        return 0;

    switch (id)
    {

        case PS2_MOUSE:
            return (control & 0x20);

    }

    return 1;

}

unsigned char ps2_getdata(void)
{

    return io_inb(REGISTERDATA);

}

unsigned char ps2_getstatus(void)
{

    return io_inb(REGISTERCONTROL);

}

unsigned short ps2_getirq(unsigned int id)
{

    return devices[id].irq;

}

void ps2_enable(unsigned int id)
{

    setcommand(devices[id].enable);

}

void ps2_disable(unsigned int id)
{

    setcommand(devices[id].disable);

}

void ps2_enableinterrupt(unsigned int id)
{

    wconfig(rconfig() | devices[id].interrupt);

}

void ps2_disableinterrupt(unsigned int id)
{

    wconfig(rconfig() & ~devices[id].interrupt);

}

void ps2_enabletranslation(unsigned int id)
{

    wconfig(rconfig() | devices[id].translation);

}

void ps2_disabletranslation(unsigned int id)
{

    wconfig(rconfig() & ~devices[id].translation);

}

void ps2_reset(unsigned int id)
{

    setdevicedata(id, COMMANDDEVRESET);
    polldata();
    polldata();

    if (id == PS2_MOUSE)
        polldata();

}

unsigned char ps2_identify(unsigned int id)
{

    setdevicedata(id, COMMANDDEVIDENTIFY);

    if (polldata() != 0xFA)
        return 0;

    return polldata();

}

void ps2_enablescanning(unsigned int id)
{

    setdevicedata(id, COMMANDDEVENABLESCAN);
    polldata();

}

void ps2_disablescanning(unsigned int id)
{

    setdevicedata(id, COMMANDDEVDISABLESCAN);
    polldata();

}

void ps2_default(unsigned int id)
{

    setdevicedata(id, COMMANDDEVDEFAULT);
    polldata();

}

void ps2_rate(unsigned int id, unsigned char rate)
{

    setdevicedata(id, COMMANDDEVRATE);

    if (polldata() != 0xFA)
        return;

    setdevicedata(id, rate);

    if (polldata() != 0xFA)
        return;

}

static void bus_setup(void)
{

    flushdata();

    if (testbus())
    {

        unsigned char config = rconfig();

        if (config & CONFIGFLAG_DEV1CLOCK)
            devices[PS2_KEYBOARD].present = testdevice(PS2_KEYBOARD);

        if (config & CONFIGFLAG_DEV2CLOCK)
            devices[PS2_MOUSE].present = testdevice(PS2_MOUSE);

    }

}

static unsigned int bus_next(unsigned int id)
{

    return (id + 1) % 3;

}

static unsigned int reset_write(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    setcommand(COMMANDCTRLRESET);

    return 0;

}

void module_init(void)
{

    base_initbus(&bus, PS2_BUS, "ps2", bus_setup, bus_next);
    system_initnode(&reset, SYSTEM_NODETYPE_NORMAL, "reset");

    reset.operations.write = reset_write;

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

