#include <module.h>
#include <kernel/resource.h>
#include <system/system.h>
#include <base/base.h>
#include <clock/clock.h>
#include <arch/x86/platform/platform.h>
#include <arch/x86/pic/pic.h>
#include <arch/x86/io/io.h>

enum rtc_register
{

    RTC_REGISTER_COMMAND                = 0x0000,
    RTC_REGISTER_DATA                   = 0x0001

};

enum rtc_flag
{

    RTC_FLAG_SECONDS                    = 0x00,
    RTC_FLAG_MINUTES                    = 0x02,
    RTC_FLAG_HOURS                      = 0x04,
    RTC_FLAG_DAY                        = 0x07,
    RTC_FLAG_MONTH                      = 0x08,
    RTC_FLAG_YEAR                       = 0x09

};

static struct base_driver driver;
static struct clock_interface clockinterface;
static unsigned short io;

static unsigned char convert(unsigned char num)
{

    return ((num & 0xF0) >> 1) + ((num & 0xF0) >> 3) + (num & 0x0F);

}

static unsigned char read(unsigned int type)
{

    io_outb(io + RTC_REGISTER_COMMAND, type);

    return convert(io_inb(io + RTC_REGISTER_DATA));

}

static void handleirq(unsigned int irq, struct base_bus *bus, unsigned int id)
{

}

static unsigned char clockinterface_getseconds(struct base_bus *bus, unsigned int id)
{

    return read(0x00);

}

static unsigned char clockinterface_getminutes(struct base_bus *bus, unsigned int id)
{

    return read(0x02);

}

static unsigned char clockinterface_gethours(struct base_bus *bus, unsigned int id)
{

    return read(0x04);

}

static unsigned char clockinterface_getweekday(struct base_bus *bus, unsigned int id)
{

    return read(0x06);

}

static unsigned char clockinterface_getday(struct base_bus *bus, unsigned int id)
{

    return read(0x07);

}

static unsigned char clockinterface_getmonth(struct base_bus *bus, unsigned int id)
{

    return read(0x08);

}

static unsigned short clockinterface_getyear(struct base_bus *bus, unsigned int id)
{

    return 2000 + read(0x09);

}

static unsigned int driver_match(struct base_bus *bus, unsigned int id)
{

    if (bus->type != PLATFORM_BUS_TYPE)
        return 0;

    return id == PLATFORM_RTC_DEVICE_TYPE;

}

static void driver_attach(struct base_bus *bus, unsigned int id)
{

    clock_initinterface(&clockinterface, &driver, bus, id, clockinterface_getseconds, clockinterface_getminutes, clockinterface_gethours, clockinterface_getweekday, clockinterface_getday, clockinterface_getmonth, clockinterface_getyear);
    clock_registerinterface(&clockinterface);
    pic_setroutine(bus, id, handleirq);

    io = platform_getbase(bus, id);

}

static void driver_detach(struct base_bus *bus, unsigned int id)
{

    pic_unsetroutine(bus, id);
    clock_unregisterinterface(&clockinterface);

}

void init()
{

    base_initdriver(&driver, "rtc", driver_match, driver_attach, driver_detach);
    base_registerdriver(&driver);

}

void destroy()
{

    base_unregisterdriver(&driver);

}

