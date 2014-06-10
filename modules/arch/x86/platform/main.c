#include <module.h>
#include <kernel/resource.h>
#include <base/base.h>
#include "platform.h"

static struct platform_bus bus;
static struct base_device pit;
static struct base_device rtc;
static struct base_device uart1;
static struct base_device uart2;
static struct base_device uart3;
static struct base_device uart4;

void init()
{

    platform_init_bus(&bus);
    base_init_device(&pit, PLATFORM_PIT_DEVICE_TYPE, "pit", &bus.base);
    base_init_device(&rtc, PLATFORM_RTC_DEVICE_TYPE, "rtc", &bus.base);
    base_init_device(&uart1, PLATFORM_UART1_DEVICE_TYPE, "uart0", &bus.base);
    base_init_device(&uart2, PLATFORM_UART2_DEVICE_TYPE, "uart1", &bus.base);
    base_init_device(&uart3, PLATFORM_UART3_DEVICE_TYPE, "uart2", &bus.base);
    base_init_device(&uart4, PLATFORM_UART4_DEVICE_TYPE, "uart3", &bus.base);
    base_register_bus(&bus.base);
    base_register_device(&pit);
    base_register_device(&rtc);
    base_register_device(&uart1);
    base_register_device(&uart2);
    base_register_device(&uart3);
    base_register_device(&uart4);

}

void destroy()
{

    base_unregister_device(&pit);
    base_unregister_device(&rtc);
    base_unregister_device(&uart1);
    base_unregister_device(&uart2);
    base_unregister_device(&uart3);
    base_unregister_device(&uart4);
    base_unregister_bus(&bus.base);

}

