#include <module.h>
#include <kernel/resource.h>
#include <system/system.h>
#include <base/base.h>
#include <base/timer.h>
#include <arch/x86/pic/pic.h>
#include <arch/x86/io/io.h>
#include <arch/x86/platform/platform.h>

#define PIT_FREQUENCY                   1193182
#define PIT_HERTZ                       1000

enum pit_register
{

    PIT_REGISTER_COUNTER0               = 0x0000,
    PIT_REGISTER_COUNTER1               = 0x0001,
    PIT_REGISTER_COUNTER2               = 0x0002,
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
    PIT_COMMAND_COUNTER0                = 0x00,
    PIT_COMMAND_COUNTER1                = 0x40,
    PIT_COMMAND_COUNTER2                = 0x80,
    PIT_COMMAND_READBACK                = 0xC0

};

static struct base_driver driver;
static struct base_timer_interface itimer;
static struct base_device device;
static struct base_timer_node node;
static unsigned short divisor;
static unsigned int jiffies;
static unsigned short io;

static void handleirq(unsigned int irq, struct base_bus *bus, unsigned int id)
{

    jiffies += 1;

}

static void itimer_addduration(struct base_bus *bus, unsigned int id, unsigned int duration)
{

}

static unsigned int driver_match(struct base_bus *bus, unsigned int id)
{

    if (bus->type != PLATFORM_BUS_TYPE)
        return 0;

    return id == PLATFORM_PIT_DEVICE_TYPE;

}

static void driver_attach(struct base_bus *bus, unsigned int id)
{

    base_initdevice(&device, bus, id);
    base_timer_initnode(&node, &device, &itimer);
    base_timer_registernode(&node);
    pic_setroutine(bus, id, handleirq);

    io = platform_getbase(bus, id);

    io_outb(io + PIT_REGISTER_COMMAND, PIT_COMMAND_COUNTER0 | PIT_COMMAND_BOTH | PIT_COMMAND_MODE3 | PIT_COMMAND_BINARY);
    io_outb(io + PIT_REGISTER_COUNTER0, divisor >> 0);
    io_outb(io + PIT_REGISTER_COUNTER0, divisor >> 8);

}

static void driver_detach(struct base_bus *bus, unsigned int id)
{

    pic_unsetroutine(bus, id);
    base_timer_unregisternode(&node);

}

void init()
{

    base_timer_initinterface(&itimer, itimer_addduration);
    base_timer_registerinterface(&itimer);
    base_initdriver(&driver, "pit", driver_match, driver_attach, driver_detach);
    base_registerdriver(&driver);

}

void destroy()
{

    base_timer_unregisterinterface(&itimer);
    base_unregisterdriver(&driver);

}

