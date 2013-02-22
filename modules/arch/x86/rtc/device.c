#include <fudge/module.h>
#include <base/base.h>
#include <arch/x86/io/io.h>
#include "rtc.h"

static unsigned char convert(unsigned char num)
{

    return ((num & 0xF0) >> 1) + ((num & 0xF0) >> 3) + (num & 0x0F);

}

static unsigned char get_value(unsigned int type)
{

    io_outb(RTC_COMMAND, type);

    return convert(io_inb(RTC_DATA));

}

void rtc_init_device(struct rtc_device *device, unsigned int irq)
{

    memory_clear(device, sizeof (struct rtc_device));
    base_init_device(&device->base, RTC_DEVICE_TYPE, irq, "rtc");

    device->get_value = get_value;

}

