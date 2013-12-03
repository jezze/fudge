#include <fudge/module.h>
#include <kernel/vfs.h>
#include <kernel/kernel.h>
#include <base/base.h>
#include <base/timer.h>
#include <arch/x86/pic/pic.h>
#include <arch/x86/io/io.h>
#include <arch/x86/platform/platform.h>
#include "driver.h"

#define PIT_FREQUENCY                   1193182
#define PIT_HERTZ                       1000

enum pit_register
{

    PIT_REGISTER_COUNTER0               = 0x00,
    PIT_REGISTER_COUNTER1               = 0x01,
    PIT_REGISTER_COUNTER2               = 0x02,
    PIT_REGISTER_COMMAND                = 0x03

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

    struct pit_driver *driver = (struct pit_driver *)device->driver;

    driver->itimer.jiffies += 1;

    kernel_rendezvous_unsleep(&driver->rduration, driver->duration + 5000 < driver->itimer.jiffies);

}

static void attach(struct base_device *device)
{

    struct platform_device *platformDevice = (struct platform_device *)device;
    struct pit_driver *driver = (struct pit_driver *)device->driver;

    base_timer_register_interface(&driver->itimer, device);
    pic_set_routine(device, handle_irq);
    io_outb(platformDevice->registers + PIT_REGISTER_COMMAND, PIT_COMMAND_COUNTER0 | PIT_COMMAND_BOTH | PIT_COMMAND_MODE3 | PIT_COMMAND_BINARY);
    io_outb(platformDevice->registers + PIT_REGISTER_COUNTER0, driver->divisor >> 0);
    io_outb(platformDevice->registers + PIT_REGISTER_COUNTER0, driver->divisor >> 8);

}

static unsigned int check(struct base_device *device)
{

    return device->type == PLATFORM_PIT_DEVICE_TYPE;

}

static void add_duration(struct base_device *device, unsigned int duration)
{

    struct pit_driver *driver = (struct pit_driver *)device->driver;

    kernel_rendezvous_sleep(&driver->rduration, 1);
    driver->duration = driver->itimer.jiffies;

}

void pit_init_driver(struct pit_driver *driver)
{

    memory_clear(driver, sizeof (struct pit_driver));
    base_init_driver(&driver->base, "pit", check, attach);
    base_timer_init_interface(&driver->itimer, add_duration);

    driver->divisor = PIT_FREQUENCY / PIT_HERTZ;

}

