#include <fudge/module.h>
#include <base/base.h>
#include <timer/timer.h>
#include <arch/x86/pic/pic.h>
#include <arch/x86/io/io.h>
#include "rtc.h"

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

static void handle_irq(struct base_device *device)
{

}

static void start(struct base_driver *self)
{

}

static void attach(struct base_device *device)
{

    pic_set_routine(device, handle_irq);

}

static unsigned int check(struct base_device *device)
{

    return device->type == RTC_DEVICE_TYPE;

}

void rtc_init_driver(struct rtc_driver *driver)
{

    memory_clear(driver, sizeof (struct rtc_driver));
    base_init_driver(&driver->base, "rtc", start, check, attach);

}

