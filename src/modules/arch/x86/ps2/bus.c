#include <fudge.h>
#include <kernel.h>
#include <modules/base/bus.h>
#include <modules/system/system.h>
#include <modules/arch/x86/io/io.h>
#include "ps2.h"

#define REG_DATA                        0x0060
#define REG_COMMAND                     0x0064
#define REG_COMMAND_CONFIGR             0x20
#define REG_COMMAND_CONFIGR_DEV1INT     (1 << 0)
#define REG_COMMAND_CONFIGR_DEV2INT     (1 << 1)
#define REG_COMMAND_CONFIGR_SYSTEM      (1 << 2)
#define REG_COMMAND_CONFIGR_DEV1CLOCK   (1 << 4)
#define REG_COMMAND_CONFIGR_DEV2CLOCK   (1 << 5)
#define REG_COMMAND_CONFIGR_DEV1TRANS   (1 << 6)
#define REG_COMMAND_CONFIGW             0x60
#define REG_COMMAND_DEV2DISABLE         0xA7
#define REG_COMMAND_DEV2ENABLE          0xA8
#define REG_COMMAND_DEV2TEST            0xA9
#define REG_COMMAND_DEV2TEST_OK         0x00
#define REG_COMMAND_DEV2TEST_CLOCKLOW   0x01
#define REG_COMMAND_DEV2TEST_CLOCKHIGH  0x02
#define REG_COMMAND_DEV2TEST_LINELOW    0x03
#define REG_COMMAND_DEV2TEST_LINEHIGH   0x04
#define REG_COMMAND_CTEST               0xAA
#define REG_COMMAND_CTEST_OK            0x55
#define REG_COMMAND_CTEST_ERROR         0xFC
#define REG_COMMAND_DEV1TEST            0xAB
#define REG_COMMAND_DEV1TEST_OK         0x00
#define REG_COMMAND_DEV1TEST_CLOCKLOW   0x01
#define REG_COMMAND_DEV1TEST_CLOCKHIGH  0x02
#define REG_COMMAND_DEV1TEST_LINELOW    0x03
#define REG_COMMAND_DEV1TEST_LINEHIGH   0x04
#define REG_COMMAND_DEV1DISABLE         0xAD
#define REG_COMMAND_DEV1ENABLE          0xAE
#define REG_COMMAND_CTRLRO              0xD0
#define REG_COMMAND_CTRLWO              0xD1
#define REG_COMMAND_DEV1WO              0xD2
#define REG_COMMAND_DEV2WO              0xD3
#define REG_COMMAND_DEV2WI              0xD4
#define REG_COMMAND_CTRLRESET           0xFE
#define REG_COMMAND_DEVIDENTIFY         0xF2
#define REG_COMMAND_DEVRATE             0xF3
#define REG_COMMAND_DEVENABLESCAN       0xF4
#define REG_COMMAND_DEVDISABLESCAN      0xF5
#define REG_COMMAND_DEVDEFAULT          0xF6
#define REG_COMMAND_DEVRESET            0xFF
#define REG_STATUS                      0x0064
#define REG_STATUS_OFULL                (1 << 0)
#define REG_STATUS_IFULL                (1 << 1)
#define REG_STATUS_SYSTEM               (1 << 2)
#define REG_STATUS_CTRL                 (1 << 3)
#define REG_STATUS_AUX                  (1 << 5)
#define REG_STATUS_TIMEOUT              (1 << 6)
#define REG_STATUS_PARITY               (1 << 7)
#define IRQ_KEYBOARD                    0x01
#define IRQ_MOUSE                       0x0C

struct device
{

    unsigned int present;
    unsigned short irq;
    unsigned char disable;
    unsigned char enable;
    unsigned char interrupt;
    unsigned char translation;
    unsigned char test;
    unsigned char testok;

};

static struct base_bus bus;
static struct system_node reset;

static struct device devices[] = {
    {0},
    {0, IRQ_KEYBOARD, REG_COMMAND_DEV1DISABLE, REG_COMMAND_DEV1ENABLE, REG_COMMAND_CONFIGR_DEV1INT, REG_COMMAND_CONFIGR_DEV1TRANS, REG_COMMAND_DEV1TEST, REG_COMMAND_DEV1TEST_OK},
    {0, IRQ_MOUSE, REG_COMMAND_DEV2DISABLE, REG_COMMAND_DEV2ENABLE, REG_COMMAND_CONFIGR_DEV2INT, 0, REG_COMMAND_DEV2TEST, REG_COMMAND_DEV2TEST_OK}
};

static void flushdata(void)
{

    while ((io_inb(REG_STATUS) & REG_STATUS_OFULL))
        io_inb(REG_DATA);

}

static unsigned char polldata(void)
{

    while (!(io_inb(REG_STATUS) & REG_STATUS_OFULL));

    return io_inb(REG_DATA);

}

static void setcommand(unsigned char value)
{

    while ((io_inb(REG_STATUS) & REG_STATUS_IFULL));

    io_outb(REG_COMMAND, value);

}

static void setdata(unsigned char value)
{

    while ((io_inb(REG_STATUS) & REG_STATUS_IFULL));

    io_outb(REG_DATA, value);

}

static void setdevicedata(unsigned int id, unsigned char value)
{

    if (id == PS2_MOUSE)
        setcommand(REG_COMMAND_DEV2WI);

    setdata(value);

}

static unsigned char rconfig(void)
{

    setcommand(REG_COMMAND_CONFIGR);

    return polldata();

}

static void wconfig(unsigned char config)
{

    setcommand(REG_COMMAND_CONFIGW);
    setdata(config);

}

static unsigned int testbus(void)
{

    setcommand(REG_COMMAND_CTEST);

    return polldata() == REG_COMMAND_CTEST_OK;

}

static unsigned int testdevice(unsigned int id)
{

    setcommand(devices[id].test);

    return polldata() == devices[id].testok;

}

unsigned int ps2_checkdata(unsigned int id)
{

    unsigned char status = io_inb(REG_STATUS);

    if ((status & REG_STATUS_TIMEOUT))
        return 0;

    if ((status & REG_STATUS_PARITY))
        return 0;

    switch (id)
    {

        case PS2_MOUSE:
            return (status & REG_STATUS_AUX);

    }

    return 1;

}

unsigned char ps2_getdata(void)
{

    return io_inb(REG_DATA);

}

unsigned char ps2_getstatus(void)
{

    return io_inb(REG_STATUS);

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

    setdevicedata(id, REG_COMMAND_DEVRESET);
    polldata();
    polldata();

    if (id == PS2_MOUSE)
        polldata();

}

unsigned char ps2_identify(unsigned int id)
{

    setdevicedata(id, REG_COMMAND_DEVIDENTIFY);

    if (polldata() != 0xFA)
        return 0;

    return polldata();

}

void ps2_enablescanning(unsigned int id)
{

    setdevicedata(id, REG_COMMAND_DEVENABLESCAN);
    polldata();

}

void ps2_disablescanning(unsigned int id)
{

    setdevicedata(id, REG_COMMAND_DEVDISABLESCAN);
    polldata();

}

void ps2_default(unsigned int id)
{

    setdevicedata(id, REG_COMMAND_DEVDEFAULT);
    polldata();

}

void ps2_rate(unsigned int id, unsigned char rate)
{

    setdevicedata(id, REG_COMMAND_DEVRATE);

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

        if (config & REG_COMMAND_CONFIGR_DEV1CLOCK)
            devices[PS2_KEYBOARD].present = testdevice(PS2_KEYBOARD);

        if (config & REG_COMMAND_CONFIGR_DEV2CLOCK)
            devices[PS2_MOUSE].present = testdevice(PS2_MOUSE);

    }

}

static unsigned int bus_next(unsigned int id)
{

    return (id + 1) % 3;

}

static unsigned int reset_notify(struct list *links, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    setcommand(REG_COMMAND_CTRLRESET);

    return 0;

}

void module_init(void)
{

    base_initbus(&bus, PS2_BUS, "ps2", bus_setup, bus_next);
    system_initnode(&reset, SYSTEM_NODETYPE_NORMAL, "reset");

    reset.operations.notify = reset_notify;

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

