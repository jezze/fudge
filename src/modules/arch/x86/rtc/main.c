#include <fudge.h>
#include <kernel.h>
#include <modules/base/driver.h>
#include <modules/system/system.h>
#include <modules/clock/clock.h>
#include <modules/arch/x86/platform/platform.h>
#include <modules/arch/x86/pic/pic.h>
#include <modules/arch/x86/io/io.h>

#define REGISTERCOMMAND                 0x0000
#define REGISTERDATA                    0x0001
#define FLAGSECONDS                     0x00
#define FLAGMINUTES                     0x02
#define FLAGHOURS                       0x04
#define FLAGDAY                         0x07
#define FLAGMONTH                       0x08
#define FLAGYEAR                        0x09

static struct base_driver driver;
static struct clock_interface clockinterface;
static unsigned short io;

static unsigned char convert(unsigned char num)
{

    return ((num & 0xF0) >> 1) + ((num & 0xF0) >> 3) + (num & 0x0F);

}

static unsigned char read(unsigned int type)
{

    io_outb(io + REGISTERCOMMAND, type);

    return convert(io_inb(io + REGISTERDATA));

}

static void handleirq(unsigned int irq)
{

}

static unsigned int clockinterface_readctrl(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    struct ctrl_clocksettings settings;

    settings.seconds = read(0x00);
    settings.minutes = read(0x02);
    settings.hours = read(0x04);
    settings.weekday = read(0x06);
    settings.day = read(0x07);
    settings.month = read(0x08);
    settings.year = 2000 + read(0x09);

    return memory_read(buffer, count, &settings, sizeof (struct ctrl_clocksettings), offset);

}

static void driver_init(unsigned int id)
{

    clock_initinterface(&clockinterface, id);

    clockinterface.ctrl.operations.read = clockinterface_readctrl;

}

static unsigned int driver_match(unsigned int id)
{

    return id == PLATFORM_RTC;

}

static void driver_reset(unsigned int id)
{

    io = platform_getbase(id);

}

static void driver_attach(unsigned int id)
{

    clock_registerinterface(&clockinterface);
    pic_setroutine(platform_getirq(id), handleirq);

}

static void driver_detach(unsigned int id)
{

    clock_unregisterinterface(&clockinterface);
    pic_unsetroutine(platform_getirq(id));

}

void module_init(void)
{

    base_initdriver(&driver, "rtc", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, PLATFORM_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, PLATFORM_BUS);

}

