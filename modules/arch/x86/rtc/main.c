#include <fudge.h>
#include <kernel.h>
#include <base/base.h>
#include <system/system.h>
#include <clock/clock.h>
#include <arch/x86/platform/platform.h>
#include <arch/x86/pic/pic.h>
#include <arch/x86/io/io.h>

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

static void handleirq(unsigned int irq, unsigned int id)
{

}

static unsigned char clockinterface_getseconds()
{

    return read(0x00);

}

static unsigned char clockinterface_getminutes()
{

    return read(0x02);

}

static unsigned char clockinterface_gethours()
{

    return read(0x04);

}

static unsigned char clockinterface_getweekday()
{

    return read(0x06);

}

static unsigned char clockinterface_getday()
{

    return read(0x07);

}

static unsigned char clockinterface_getmonth()
{

    return read(0x08);

}

static unsigned short clockinterface_getyear()
{

    return 2000 + read(0x09);

}

static void driver_init()
{

    clock_initinterface(&clockinterface, clockinterface_getseconds, clockinterface_getminutes, clockinterface_gethours, clockinterface_getweekday, clockinterface_getday, clockinterface_getmonth, clockinterface_getyear);

}

static unsigned int driver_match(unsigned int id)
{

    return id == PLATFORM_RTC;

}

static void driver_attach(unsigned int id)
{

    io = platform_getbase(id);

    clock_registerinterface(&clockinterface, id);
    pic_setroutine(platform_getirq(id), id, handleirq);

}

static void driver_detach(unsigned int id)
{

    clock_unregisterinterface(&clockinterface);
    pic_unsetroutine(platform_getirq(id), id);

}

void module_init()
{

    base_initdriver(&driver, "rtc", driver_init, driver_match, driver_attach, driver_detach);

}

void module_register()
{

    base_registerdriver(&driver, PLATFORM_BUS);

}

void module_unregister()
{

    base_unregisterdriver(&driver, PLATFORM_BUS);

}

