#include <fudge/module.h>
#include <base/base.h>
#include <timer/timer.h>
#include <arch/x86/pic/pic.h>
#include <arch/x86/io/io.h>
#include "pit.h"

enum pit_register
{

    PIT_REGISTER_COUNTER0               = 0x40,
    PIT_REGISTER_COUNTER1               = 0x41,
    PIT_REGISTER_COUNTER2               = 0x42,
    PIT_REGISTER_COMMAND                = 0x43

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

static void handle_irq(struct base_device *device)
{

    struct pit_device *pitDevice = (struct pit_device *)device;

    pitDevice->jiffies += 1;

}

static void attach(struct base_device *device)
{

    struct pit_device *pitDevice = (struct pit_device *)device;

    pic_set_routine(device, handle_irq);
    io_outb(PIT_REGISTER_COMMAND, PIT_COMMAND_COUNTER0 | PIT_COMMAND_BOTH | PIT_COMMAND_MODE3 | PIT_COMMAND_BINARY);
    io_outb(PIT_REGISTER_COUNTER0, pitDevice->divisor >> 0);
    io_outb(PIT_REGISTER_COUNTER0, pitDevice->divisor >> 8);

}

static unsigned int check(struct base_device *device)
{

    return device->type == PIT_DEVICE_TYPE;

}

void pit_init_driver(struct pit_driver *driver)
{

    memory_clear(driver, sizeof (struct pit_driver));
    base_init_driver(&driver->base, "pit", 0, check, attach);
    timer_init_interface(&driver->itimer, &driver->base);

}

