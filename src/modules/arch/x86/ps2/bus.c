#include <fudge.h>
#include <kernel.h>
#include <modules/base/bus.h>
#include <modules/system/system.h>
#include <modules/arch/x86/io/io.h>
#include "ps2.h"

#define I_KEYBOARD                      0x01
#define I_MOUSE                         0x0C
#define R_DATA                          0x0060
#define R_COMMAND                       0x0064
#define R_STATUS                        0x0064
#define C_COMMAND_CONFIGR               0x20
#define C_COMMAND_CONFIGW               0x60
#define C_COMMAND_DEV2DISABLE           0xA7
#define C_COMMAND_DEV2ENABLE            0xA8
#define C_COMMAND_DEV2TEST              0xA9
#define C_COMMAND_CTEST                 0xAA
#define C_COMMAND_DEV1TEST              0xAB
#define C_COMMAND_DEV1DISABLE           0xAD
#define C_COMMAND_DEV1ENABLE            0xAE
#define C_COMMAND_CTRLRO                0xD0
#define C_COMMAND_CTRLWO                0xD1
#define C_COMMAND_DEV1WO                0xD2
#define C_COMMAND_DEV2WO                0xD3
#define C_COMMAND_DEV2WI                0xD4
#define C_COMMAND_CTRLRESET             0xFE
#define C_COMMAND_DEVIDENTIFY           0xF2
#define C_COMMAND_DEVRATE               0xF3
#define C_COMMAND_DEVENABLESCAN         0xF4
#define C_COMMAND_DEVDISABLESCAN        0xF5
#define C_COMMAND_DEVDEFAULT            0xF6
#define C_COMMAND_DEVRESET              0xFF
#define C_COMMAND_CTEST_OK              0x55
#define C_COMMAND_CTEST_ERROR           0xFC
#define F_COMMAND_CONFIGR_DEV1INT       (1 << 0)
#define F_COMMAND_CONFIGR_DEV2INT       (1 << 1)
#define F_COMMAND_CONFIGR_SYSTEM        (1 << 2)
#define F_COMMAND_CONFIGR_DEV1CLOCK     (1 << 4)
#define F_COMMAND_CONFIGR_DEV2CLOCK     (1 << 5)
#define F_COMMAND_CONFIGR_DEV1TRANS     (1 << 6)
#define C_COMMAND_DEVTEST_OK            0x00
#define C_COMMAND_DEVTEST_CLOCKLOW      0x01
#define C_COMMAND_DEVTEST_CLOCKHIGH     0x02
#define C_COMMAND_DEVTEST_LINELOW       0x03
#define C_COMMAND_DEVTEST_LINEHIGH      0x04
#define F_STATUS_OFULL                  (1 << 0)
#define F_STATUS_IFULL                  (1 << 1)
#define F_STATUS_SYSTEM                 (1 << 2)
#define F_STATUS_CTRL                   (1 << 3)
#define F_STATUS_AUX                    (1 << 5)
#define F_STATUS_TIMEOUT                (1 << 6)
#define F_STATUS_PARITY                 (1 << 7)

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
    {0, I_KEYBOARD, C_COMMAND_DEV1DISABLE, C_COMMAND_DEV1ENABLE, F_COMMAND_CONFIGR_DEV1INT, F_COMMAND_CONFIGR_DEV1TRANS, C_COMMAND_DEV1TEST},
    {0, I_MOUSE, C_COMMAND_DEV2DISABLE, C_COMMAND_DEV2ENABLE, F_COMMAND_CONFIGR_DEV2INT, 0, C_COMMAND_DEV2TEST}
};

static void flushdata(void)
{

    while ((io_inb(R_STATUS) & F_STATUS_OFULL))
        io_inb(R_DATA);

}

static unsigned char polldata(void)
{

    while (!(io_inb(R_STATUS) & F_STATUS_OFULL));

    return io_inb(R_DATA);

}

static void setcommand(unsigned char value)
{

    while ((io_inb(R_STATUS) & F_STATUS_IFULL));

    io_outb(R_COMMAND, value);

}

static void setdata(unsigned char value)
{

    while ((io_inb(R_STATUS) & F_STATUS_IFULL));

    io_outb(R_DATA, value);

}

static void setdevicedata(unsigned int id, unsigned char value)
{

    if (id == PS2_MOUSE)
        setcommand(C_COMMAND_DEV2WI);

    setdata(value);

}

static unsigned char rconfig(void)
{

    setcommand(C_COMMAND_CONFIGR);

    return polldata();

}

static void wconfig(unsigned char config)
{

    setcommand(C_COMMAND_CONFIGW);
    setdata(config);

}

static unsigned int testbus(void)
{

    setcommand(C_COMMAND_CTEST);

    return polldata() == C_COMMAND_CTEST_OK;

}

static unsigned int testdevice(unsigned int id)
{

    setcommand(devices[id].test);

    return polldata() == C_COMMAND_DEVTEST_OK;

}

unsigned int ps2_checkdata(unsigned int id)
{

    unsigned char status = io_inb(R_STATUS);

    if ((status & F_STATUS_TIMEOUT))
        return 0;

    if ((status & F_STATUS_PARITY))
        return 0;

    switch (id)
    {

        case PS2_MOUSE:
            return (status & F_STATUS_AUX);

    }

    return 1;

}

unsigned char ps2_getdata(void)
{

    return io_inb(R_DATA);

}

unsigned char ps2_getstatus(void)
{

    return io_inb(R_STATUS);

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

    setdevicedata(id, C_COMMAND_DEVRESET);
    polldata();
    polldata();

    if (id == PS2_MOUSE)
        polldata();

}

unsigned char ps2_identify(unsigned int id)
{

    setdevicedata(id, C_COMMAND_DEVIDENTIFY);

    if (polldata() != 0xFA)
        return 0;

    return polldata();

}

void ps2_enablescanning(unsigned int id)
{

    setdevicedata(id, C_COMMAND_DEVENABLESCAN);
    polldata();

}

void ps2_disablescanning(unsigned int id)
{

    setdevicedata(id, C_COMMAND_DEVDISABLESCAN);
    polldata();

}

void ps2_default(unsigned int id)
{

    setdevicedata(id, C_COMMAND_DEVDEFAULT);
    polldata();

}

void ps2_rate(unsigned int id, unsigned char rate)
{

    setdevicedata(id, C_COMMAND_DEVRATE);

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

        if (config & F_COMMAND_CONFIGR_DEV1CLOCK)
            devices[PS2_KEYBOARD].present = testdevice(PS2_KEYBOARD);

        if (config & F_COMMAND_CONFIGR_DEV2CLOCK)
            devices[PS2_MOUSE].present = testdevice(PS2_MOUSE);

    }

}

static unsigned int bus_next(unsigned int id)
{

    return (id + 1) % 3;

}

static unsigned int reset_write(void *buffer, unsigned int count, unsigned int offset)
{

    setcommand(C_COMMAND_CTRLRESET);

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

