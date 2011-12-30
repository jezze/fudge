#include <kernel/modules.h>
#include <modules/rtc/rtc.h>

static struct rtc_device device;

void init()
{

    rtc_device_init(&device);

    modules_register_device(&device.base);

}

void destroy()
{

    modules_unregister_device(&device.base);

}

