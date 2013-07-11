#include <fudge/module.h>
#include <base/base.h>
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

void rtc_init_device(struct rtc_device *device, unsigned int irq)
{

    memory_clear(device, sizeof (struct rtc_device));
    base_init_device(&device->base, RTC_DEVICE_TYPE, irq, "rtc", 0);

    device->get_value = get_value;

}

