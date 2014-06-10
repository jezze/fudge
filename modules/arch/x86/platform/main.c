#include <module.h>
#include <kernel/resource.h>
#include <base/base.h>
#include "platform.h"

static struct platform_bus bus;
static struct platform_device pit;
static struct platform_device rtc;
static struct platform_device uart1;
static struct platform_device uart2;
static struct platform_device uart3;
static struct platform_device uart4;

void init()
{

    platform_init_bus(&bus);
    platform_init_device(&pit, PLATFORM_PIT_DEVICE_TYPE, "pit", &bus, 0x0040);
    platform_init_device(&rtc, PLATFORM_RTC_DEVICE_TYPE, "rtc", &bus, 0x0070);
    platform_init_device(&uart1, PLATFORM_UART1_DEVICE_TYPE, "uart0", &bus, 0x03F8);
    platform_init_device(&uart2, PLATFORM_UART2_DEVICE_TYPE, "uart1", &bus, 0x02F8);
    platform_init_device(&uart3, PLATFORM_UART1_DEVICE_TYPE, "uart2", &bus, 0x03E8);
    platform_init_device(&uart4, PLATFORM_UART2_DEVICE_TYPE, "uart3", &bus, 0x02E8);
    base_register_bus(&bus.base);
    base_register_device(&pit.base);
    base_register_device(&rtc.base);
    base_register_device(&uart1.base);
    base_register_device(&uart2.base);
    base_register_device(&uart3.base);
    base_register_device(&uart4.base);

}

void destroy()
{

    base_unregister_device(&pit.base);
    base_unregister_device(&rtc.base);
    base_unregister_device(&uart1.base);
    base_unregister_device(&uart2.base);
    base_unregister_device(&uart3.base);
    base_unregister_device(&uart4.base);
    base_unregister_bus(&bus.base);

}

