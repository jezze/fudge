#include <fudge/module.h>
#include <base/base.h>
#include <base/clock.h>
#include <arch/x86/platform/platform.h>
#include <arch/x86/pic/pic.h>
#include <arch/x86/io/io.h>
#include "driver.h"

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

static unsigned char convert(unsigned char num)
{

    return ((num & 0xF0) >> 1) + ((num & 0xF0) >> 3) + (num & 0x0F);

}

static unsigned char get_value(unsigned int type)
{

    io_outb(RTC_REGISTER_COMMAND, type);

    return convert(io_inb(RTC_REGISTER_DATA));

}

static unsigned char get_seconds(struct base_device *device)
{

    return get_value(0x00);

}

static unsigned char get_minutes(struct base_device *device)
{

    return get_value(0x02);

}

static unsigned char get_hours(struct base_device *device)
{

    return get_value(0x04);

}

static unsigned char get_weekday(struct base_device *device)
{

    return get_value(0x06);

}

static unsigned char get_day(struct base_device *device)
{

    return get_value(0x07);

}

static unsigned char get_month(struct base_device *device)
{

    return get_value(0x08);

}

static unsigned short get_year(struct base_device *device)
{

    return 2000 + get_value(0x09);

}

static void handle_irq(struct base_device *device)
{

}

static void attach(struct base_device *device)
{

    struct rtc_driver *driver = (struct rtc_driver *)device->driver;

    base_clock_register_interface(&driver->iclock, device);
    pic_set_routine(device, handle_irq);

}

static unsigned int check(struct base_device *device)
{

    return device->type == PLATFORM_RTC_DEVICE_TYPE;

}

void rtc_init_driver(struct rtc_driver *driver)
{

    memory_clear(driver, sizeof (struct rtc_driver));
    base_init_driver(&driver->base, "rtc", check, attach);
    base_clock_init_interface(&driver->iclock, get_seconds, get_minutes, get_hours, get_weekday, get_day, get_month, get_year);

}

