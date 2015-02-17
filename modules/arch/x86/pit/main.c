#include <fudge.h>
#include <kernel.h>
#include <system/system.h>
#include <base/base.h>
#include <timer/timer.h>
#include <arch/x86/pic/pic.h>
#include <arch/x86/io/io.h>
#include <arch/x86/platform/platform.h>

#define PIT_FREQUENCY                   1193182

enum pit_register
{

    PIT_REGISTER_CHANNEL0               = 0x0000,
    PIT_REGISTER_CHANNEL1               = 0x0001,
    PIT_REGISTER_CHANNEL2               = 0x0002,
    PIT_REGISTER_COMMAND                = 0x0003

};

enum pit_command
{

    PIT_COMMAND_BINARY                  = 0x00,
    PIT_COMMAND_BCD                     = 0x01,
    PIT_COMMAND_MODE0                   = 0x00,
    PIT_COMMAND_MODE1                   = 0x02,
    PIT_COMMAND_MODE2                   = 0x04,
    PIT_COMMAND_MODE3                   = 0x06,
    PIT_COMMAND_MODE4                   = 0x08,
    PIT_COMMAND_MODE5                   = 0x0A,
    PIT_COMMAND_LATCH                   = 0x00,
    PIT_COMMAND_LOW                     = 0x10,
    PIT_COMMAND_HIGH                    = 0x20,
    PIT_COMMAND_BOTH                    = 0x30,
    PIT_COMMAND_CHANNEL0                = 0x00,
    PIT_COMMAND_CHANNEL1                = 0x40,
    PIT_COMMAND_CHANNEL2                = 0x80,
    PIT_COMMAND_READBACK                = 0xC0

};

static struct base_driver driver;
static struct timer_interface timerinterface;
static unsigned short io;
static unsigned short divisor;
static unsigned int jiffies;

static void handleirq(unsigned int irq, struct base_bus *bus, unsigned int id)
{

    jiffies += 1;

    if (jiffies < 1000)
        return;

    timer_notify(&timerinterface, 4, &jiffies);

    jiffies = 0;

}

static void driver_init(struct base_driver *self)
{

    jiffies = 0;
    divisor = 1000;

    timer_initinterface(&timerinterface, self);

}

static unsigned int driver_match(struct base_driver *self, struct base_bus *bus, unsigned int id)
{

    if (bus->type != PLATFORM_BUS_TYPE)
        return 0;

    return id == PLATFORM_PIT_DEVICE_TYPE;

}

static void driver_attach(struct base_driver *self, struct base_bus *bus, unsigned int id)
{

    io = platform_getbase(bus, id);

    io_outb(io + PIT_REGISTER_COMMAND, PIT_COMMAND_CHANNEL0 | PIT_COMMAND_BOTH | PIT_COMMAND_MODE3 | PIT_COMMAND_BINARY);
    io_outb(io + PIT_REGISTER_CHANNEL0, divisor);
    io_outb(io + PIT_REGISTER_CHANNEL0, divisor >> 8);
    timer_registerinterface(&timerinterface, bus, id);
    pic_setroutine(bus, id, handleirq);

}

static void driver_detach(struct base_driver *self, struct base_bus *bus, unsigned int id)
{

    timer_unregisterinterface(&timerinterface);
    pic_unsetroutine(bus, id);

}

void module_init()
{

    base_initdriver(&driver, "pit", driver_init, driver_match, driver_attach, driver_detach);

}

void module_register()
{

    base_registerdriver(&driver);

}

void module_unregister()
{

    base_unregisterdriver(&driver);

}

