#include <fudge/module.h>
#include <system/system.h>
#include <base/base.h>
#include "rtc.h"

void rtc_init_device(struct rtc_device *device, unsigned int irq)
{

    memory_clear(device, sizeof (struct rtc_device));
    base_init_device(&device->base, RTC_DEVICE_TYPE, irq, "rtc", 0);

}

