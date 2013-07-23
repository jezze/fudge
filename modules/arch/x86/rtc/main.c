#include <fudge/module.h>
#include <base/base.h>
#include "rtc.h"

static struct rtc_device device;
static struct rtc_driver driver;

void init()
{

    rtc_init_device(&device, RTC_IRQ);
    base_register_device(&device.base);
    rtc_init_driver(&driver);
    base_register_driver(&driver.base);

}

void destroy()
{

    base_unregister_driver(&driver.base);
    base_unregister_device(&device.base);

}

