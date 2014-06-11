#include <module.h>
#include <kernel/resource.h>
#include <base/base.h>
#include "platform.h"

enum platform_register
{

    PLATFORM_REGISTER_PIT               = 0x0040,
    PLATFORM_REGISTER_RTC               = 0x0070,
    PLATFORM_REGISTER_UART1             = 0x03F8,
    PLATFORM_REGISTER_UART2             = 0x02F8,
    PLATFORM_REGISTER_UART3             = 0x03E8,
    PLATFORM_REGISTER_UART4             = 0x02E8

};

enum platform_irq
{

    PLATFORM_IRQ_PIT                    = 0x00,
    PLATFORM_IRQ_RTC                    = 0x08,
    PLATFORM_IRQ_UART1                  = 0x04,
    PLATFORM_IRQ_UART2                  = 0x03

};

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
            return PLATFORM_REGISTER_PIT;

        case PLATFORM_RTC_DEVICE_TYPE:
            return PLATFORM_REGISTER_RTC;

        case PLATFORM_UART1_DEVICE_TYPE:
            return PLATFORM_REGISTER_UART1;

        case PLATFORM_UART2_DEVICE_TYPE:
            return PLATFORM_REGISTER_UART2;

        case PLATFORM_UART3_DEVICE_TYPE:
            return PLATFORM_REGISTER_UART3;

        case PLATFORM_UART4_DEVICE_TYPE:
            return PLATFORM_REGISTER_UART4;

    }

    return 0xFFFF;

}

static void setup(struct base_bus *self)
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

static unsigned short device_irq(struct base_bus *self, unsigned int id)
{

    switch (id)
    {

        case PLATFORM_PIT_DEVICE_TYPE:
            return PLATFORM_IRQ_PIT;

        case PLATFORM_RTC_DEVICE_TYPE:
            return PLATFORM_IRQ_RTC;

        case PLATFORM_UART1_DEVICE_TYPE:
            return PLATFORM_IRQ_UART1;

        case PLATFORM_UART2_DEVICE_TYPE:
            return PLATFORM_IRQ_UART2;

        case PLATFORM_UART3_DEVICE_TYPE:
            return PLATFORM_IRQ_UART1;

        case PLATFORM_UART4_DEVICE_TYPE:
            return PLATFORM_IRQ_UART2;

    }

    return 0xFFFF;

}

void init()
{

    base_init_bus(&bus, "platform", setup, device_irq);
    base_register_bus(&bus);

}

void destroy()
{

    base_unregister_bus(&bus);

}

