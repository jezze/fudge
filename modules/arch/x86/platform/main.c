#include <module.h>
#include <kernel/resource.h>
#include <base/base.h>
#include "platform.h"

#define PLATFORM_PIT_IRQ                0x00
#define PLATFORM_RTC_IRQ                0x08
#define PLATFORM_UART1_IRQ              0x04
#define PLATFORM_UART2_IRQ              0x03
#define PLATFORM_UART3_IRQ              0x04
#define PLATFORM_UART4_IRQ              0x03

static struct base_bus bus;
static struct base_device pit;
static struct base_device rtc;
static struct base_device uart1;
static struct base_device uart2;
static struct base_device uart3;
static struct base_device uart4;

unsigned short platform_bus_get_base(struct base_bus *bus, unsigned int type)
{

    switch (type)
    {

        case PLATFORM_PIT_DEVICE_TYPE:
            return 0x0040;

        case PLATFORM_RTC_DEVICE_TYPE:
            return 0x0070;

        case PLATFORM_UART1_DEVICE_TYPE:
            return 0x03F8;

        case PLATFORM_UART2_DEVICE_TYPE:
            return 0x02F8;

        case PLATFORM_UART3_DEVICE_TYPE:
            return 0x03E8;

        case PLATFORM_UART4_DEVICE_TYPE:
            return 0x02E8;

    }

    return 0xFFFF;

}

static void scan(struct base_bus *self)
{

    base_init_device(&pit, PLATFORM_PIT_DEVICE_TYPE, "pit", self);
    base_register_device(&pit);
    base_init_device(&rtc, PLATFORM_RTC_DEVICE_TYPE, "rtc", self);
    base_register_device(&rtc);
    base_init_device(&uart1, PLATFORM_UART1_DEVICE_TYPE, "uart0", self);
    base_register_device(&uart1);
    base_init_device(&uart2, PLATFORM_UART2_DEVICE_TYPE, "uart1", self);
    base_register_device(&uart2);
    base_init_device(&uart3, PLATFORM_UART3_DEVICE_TYPE, "uart2", self);
    base_register_device(&uart3);
    base_init_device(&uart4, PLATFORM_UART4_DEVICE_TYPE, "uart3", self);
    base_register_device(&uart4);

}

static unsigned short device_irq(struct base_bus *self, struct base_device *device)
{

    switch (device->type)
    {

        case PLATFORM_PIT_DEVICE_TYPE:
            return PLATFORM_PIT_IRQ;

        case PLATFORM_RTC_DEVICE_TYPE:
            return PLATFORM_RTC_IRQ;

        case PLATFORM_UART1_DEVICE_TYPE:
            return PLATFORM_UART1_IRQ;

        case PLATFORM_UART2_DEVICE_TYPE:
            return PLATFORM_UART2_IRQ;

        case PLATFORM_UART3_DEVICE_TYPE:
            return PLATFORM_UART3_IRQ;

        case PLATFORM_UART4_DEVICE_TYPE:
            return PLATFORM_UART4_IRQ;

    }

    return 0xFFFF;

}

void init()
{

    base_init_bus(&bus, "platform", scan, device_irq);
    base_register_bus(&bus);

}

void destroy()
{

    base_unregister_bus(&bus);

}

