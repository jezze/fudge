#include <module.h>
#include <kernel/resource.h>
#include <base/base.h>
#include <base/clock.h>
#include <arch/x86/platform/platform.h>
#include <arch/x86/pic/pic.h>
#include <arch/x86/io/io.h>

enum rtc_register
{

    RTC_REGISTER_COMMAND                = 0x0070,
    RTC_REGISTER_DATA                   = 0x0071

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
static struct base_clock_interface iclock;

static unsigned char convert(unsigned char num)
{

    return ((num & 0xF0) >> 1) + ((num & 0xF0) >> 3) + (num & 0x0F);

}

static unsigned char get_value(unsigned int type)
{

    io_outb(RTC_REGISTER_COMMAND, type);

    return convert(io_inb(RTC_REGISTER_DATA));

}

static unsigned char get_seconds(struct base_bus *bus, unsigned int id)
{

    return get_value(0x00);

}

static unsigned char get_minutes(struct base_bus *bus, unsigned int id)
{

    return get_value(0x02);

}

static unsigned char get_hours(struct base_bus *bus, unsigned int id)
{

    return get_value(0x04);

}

static unsigned char get_weekday(struct base_bus *bus, unsigned int id)
{

    return get_value(0x06);

}

static unsigned char get_day(struct base_bus *bus, unsigned int id)
{

    return get_value(0x07);

}

static unsigned char get_month(struct base_bus *bus, unsigned int id)
{

    return get_value(0x08);

}

static unsigned short get_year(struct base_bus *bus, unsigned int id)
{

    return 2000 + get_value(0x09);

}

static void handle_irq(unsigned int irq, struct base_bus *bus, unsigned int id)
{

}

static unsigned int check(struct base_bus *bus, unsigned int id)
{

    if (bus->type != PLATFORM_BUS_TYPE)
        return 0;

    return id == PLATFORM_RTC_DEVICE_TYPE;

}

static void attach(struct base_bus *bus, unsigned int id)
{

    unsigned short irq = bus->device_irq(bus, id);

    base_clock_connect_interface(&iclock, bus, id);
    pic_set_routine(irq, bus, id, handle_irq);

}

static void detach(struct base_bus *bus, unsigned int id)
{

    unsigned short irq = bus->device_irq(bus, id);

    base_clock_unregister_interface(&iclock);
    pic_unset_routine(irq, bus, id);

}

void init()
{

    base_clock_init_interface(&iclock, get_seconds, get_minutes, get_hours, get_weekday, get_day, get_month, get_year);
    base_clock_register_interface(&iclock);
    base_init_driver(&driver, "rtc", check, attach, detach);
    base_register_driver(&driver);

}

void destroy()
{

    base_unregister_driver(&driver);

}

