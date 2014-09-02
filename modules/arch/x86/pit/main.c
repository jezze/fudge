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

static void add_duration(struct base_bus *bus, unsigned int id, unsigned int duration)
{

}

static void handle_irq(unsigned int irq, struct base_bus *bus, unsigned int id)
{

    jiffies += 1;

}

static unsigned int check(struct base_bus *bus, unsigned int id)
{

    if (bus->type != PLATFORM_BUS_TYPE)
        return 0;

    return id == PLATFORM_PIT_DEVICE_TYPE;

}

static void attach(struct base_bus *bus, unsigned int id)
{

    base_init_device(&device, bus, id);
    base_timer_init_node(&node, &device, &itimer);
    base_timer_register_node(&node);
    pic_set_routine(bus, id, handle_irq);

    io = platform_get_base(bus, id);

    io_outb(io + PIT_REGISTER_COMMAND, PIT_COMMAND_COUNTER0 | PIT_COMMAND_BOTH | PIT_COMMAND_MODE3 | PIT_COMMAND_BINARY);
    io_outb(io + PIT_REGISTER_COUNTER0, divisor >> 0);
    io_outb(io + PIT_REGISTER_COUNTER0, divisor >> 8);

}

static void detach(struct base_bus *bus, unsigned int id)
{

    pic_unset_routine(bus, id);
    base_timer_unregister_node(&node);

}

void init()
{

    base_timer_init_interface(&itimer, add_duration);
    base_timer_register_interface(&itimer);
    base_init_driver(&driver, "pit", check, attach, detach);
    base_register_driver(&driver);

}

void destroy()
{

    base_timer_unregister_interface(&itimer);
    base_unregister_driver(&driver);

}

