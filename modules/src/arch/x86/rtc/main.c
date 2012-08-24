#include <memory.h>
#include <modules/modules.h>
#include <arch/x86/rtc/rtc.h>

static struct rtc_device device;

void init()
{

    rtc_device_init(&device, RTC_IRQ);
    modules_register_device(&device.base);

}

void destroy()
{

    modules_unregister_device(&device.base);

}

